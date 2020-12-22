#include "posix.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "apf.h"

struct arg_ctx {
	PyObject *argv;
};


static struct apf_arg
sym_cb(struct apf_err_ctx *err, void *_ctx, const char *sym, uint16_t len)
{
	/* struct arg_ctx *ctx = _ctx; */
	return apf_tag("jej");
}

static struct apf_arg
id_cb(struct apf_err_ctx *err, void *_ctx, uint16_t id)
{
	struct arg_ctx *ctx = _ctx;

	if (id > PyList_Size(ctx->argv)) {
		err->err = apf_err_arg_missing;
		return apf_arg_null;
	}

	PyObject *itm = PyList_GetItem(ctx->argv, id);
	if (!itm) {
		err->err = apf_err_arg_missing;
		return apf_arg_null;
	}

	if (PyLong_Check(itm)) {
		Py_ssize_t num = PyNumber_AsSsize_t(itm, NULL);
		/* TODO: check for overflow? */
		return apf_tag((int32_t)num);
	} else if (PyFloat_Check(itm)) {
		double dub = PyFloat_AS_DOUBLE(itm);
		return apf_tag((float)dub);
	} else if (PyUnicode_Check(itm)) {
		const char *str = PyUnicode_AsUTF8(itm);

		if (str) {
			return apf_tag(str);
		} else {
			return apf_tag("unicode decode error");
		}
	} else {
		return apf_tag("unsupported argument type");
	}
}

#define ELEM_BLEN 512
#define BUF_BLEN 4096

static PyObject *
apf(PyObject *self, PyObject *args)
{
	const char *fmt;
	PyObject *fmtargs;

	if (!PyArg_ParseTuple(args, "sO", &fmt, &fmtargs)) {
		goto err_1;
	}

	uint8_t elems[ELEM_BLEN] = { 0 };
	char buf[BUF_BLEN] = { 0 };
	struct apf_err_ctx err = { 0 };
	struct apf_template apft;

	apft = apf_compile(elems, ELEM_BLEN, fmt, NULL, NULL, &err);
	if (err.err) {
		goto err_2;
	}

	struct arg_ctx arg_ctx = { .argv = fmtargs };
	apf_fmt(buf, BUF_BLEN, &apft, &arg_ctx, id_cb, sym_cb, &err);
	if (err.err) {
		goto err_2;
	}

	return Py_BuildValue("s", buf);
err_1:
	PyErr_SetString(PyExc_RuntimeError, "PyArg_ParseTuple failed");
	return NULL;
err_2:
	apf_strerr(buf, BUF_BLEN, &err);
	PyErr_SetString(PyExc_RuntimeError, buf);
	return NULL;
}

static PyMethodDef apf_methods[] = {
	{ "apf",  apf, METH_VARARGS, "altprintf" },
	{ NULL, NULL, 0, NULL }  /* Sentinel */
};

static struct PyModuleDef apf_module = {
	PyModuleDef_HEAD_INIT,
	"apf", /* name of module */
	NULL,  /* module documentation, may be NULL */
	-1,    /* size of per-interpreter state of the module,
	          or -1 if the module keeps state in global variables. */
	apf_methods
};

PyMODINIT_FUNC
PyInit_apf(void)
{
	return PyModule_Create(&apf_module);
}
