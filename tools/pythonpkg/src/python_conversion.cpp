#include "duckdb_python/python_conversion.hpp"
#include "duckdb_python/pybind_wrapper.hpp"

#include "datetime.h" //From Python

namespace duckdb {

Value TransformPythonValue(py::handle ele) {
	auto datetime_mod = py::module::import("datetime");
	auto datetime_date = datetime_mod.attr("date");
	auto datetime_datetime = datetime_mod.attr("datetime");
	auto datetime_time = datetime_mod.attr("time");
	auto decimal_mod = py::module::import("decimal");
	auto decimal_decimal = decimal_mod.attr("Decimal");
	auto numpy_mod = py::module::import("numpy");
	auto numpy_ndarray = numpy_mod.attr("ndarray");
	auto uuid_mod = py::module::import("uuid");
	auto uuid_uuid = uuid_mod.attr("UUID");

	if (ele.is_none()) {
		return Value();
	} else if (py::isinstance<py::bool_>(ele)) {
		return Value::BOOLEAN(ele.cast<bool>());
	} else if (py::isinstance<py::int_>(ele)) {
		return Value::BIGINT(ele.cast<int64_t>());
	} else if (py::isinstance<py::float_>(ele)) {
		if (std::isnan(PyFloat_AsDouble(ele.ptr()))) {
			return Value();
		}
		return Value::DOUBLE(ele.cast<double>());
	} else if (py::isinstance(ele, decimal_decimal)) {
		return py::str(ele).cast<string>();
	} else if (py::isinstance(ele, uuid_uuid)) {
		auto string_val = py::str(ele).cast<string>();
		return Value::UUID(string_val);
	} else if (py::isinstance(ele, datetime_datetime)) {
		auto ptr = ele.ptr();
		auto year = PyDateTime_GET_YEAR(ptr);
		auto month = PyDateTime_GET_MONTH(ptr);
		auto day = PyDateTime_GET_DAY(ptr);
		auto hour = PyDateTime_DATE_GET_HOUR(ptr);
		auto minute = PyDateTime_DATE_GET_MINUTE(ptr);
		auto second = PyDateTime_DATE_GET_SECOND(ptr);
		auto micros = PyDateTime_DATE_GET_MICROSECOND(ptr);
		return Value::TIMESTAMP(year, month, day, hour, minute, second, micros);
	} else if (py::isinstance(ele, datetime_time)) {
		auto ptr = ele.ptr();
		auto hour = PyDateTime_TIME_GET_HOUR(ptr);
		auto minute = PyDateTime_TIME_GET_MINUTE(ptr);
		auto second = PyDateTime_TIME_GET_SECOND(ptr);
		auto micros = PyDateTime_TIME_GET_MICROSECOND(ptr);
		return Value::TIME(hour, minute, second, micros);
	} else if (py::isinstance(ele, datetime_date)) {
		auto ptr = ele.ptr();
		auto year = PyDateTime_GET_YEAR(ptr);
		auto month = PyDateTime_GET_MONTH(ptr);
		auto day = PyDateTime_GET_DAY(ptr);
		return Value::DATE(year, month, day);
	} else if (py::isinstance<py::str>(ele)) {
		return ele.cast<string>();
	} else if (py::isinstance<py::bytearray>(ele)) {
		auto byte_array = ele.ptr();
		auto bytes = PyByteArray_AsString(byte_array);
		return Value::BLOB_RAW(bytes);
	} else if (py::isinstance<py::memoryview>(ele)) {
		py::memoryview py_view = ele.cast<py::memoryview>();
		PyObject *py_view_ptr = py_view.ptr();
		Py_buffer *py_buf = PyMemoryView_GET_BUFFER(py_view_ptr);
		return Value::BLOB(const_data_ptr_t(py_buf->buf), idx_t(py_buf->len));
	} else if (py::isinstance<py::bytes>(ele)) {
		const string &ele_string = ele.cast<string>();
		return Value::BLOB(const_data_ptr_t(ele_string.data()), ele_string.size());
	} else if (py::isinstance<py::list>(ele)) {
		auto size = py::len(ele);

		if (size == 0) {
			return Value::EMPTYLIST(LogicalType::SQLNULL);
		}

		vector<Value> values;
		values.reserve(size);

		bool first = true;
		for (auto py_val : ele) {
			Value new_value = TransformPythonValue(py_val);
			// First value does not need to be checked
			// If a value is NULL, it's always fine
			// Otherwise if the types dont match, throw an exception
			if (!first && !new_value.IsNull() && values[0].type().id() != new_value.type().id()) {
				throw std::runtime_error("TransformPythonValue - list contains elements of differing types");
			}
			values.push_back(move(new_value));
			first = false;
		}

		return Value::LIST(values);
	} else if (py::isinstance<py::dict>(ele)) {
		// keys() = [key, value]
		// values() = [ ..keys.. ], [ ..values.. ]
		auto dict_keys = py::list(ele.attr("keys")());
		auto dict_values = py::list(ele.attr("values")());
		auto key_size = py::len(dict_keys);
		auto value_size = py::len(dict_values);

		if (key_size != value_size) {
			throw std::runtime_error("Dictionary malformed, should have 2 'dict.values', a list of keys(index 0) and a "
			                         "list of values(index 1).");
		}
		auto keys = dict_values.attr("__getitem__")(0);
		auto values = dict_values.attr("__getitem__")(1);
		// Dont check for 'py::list' to allow ducktyping
		if (!py::hasattr(keys, "__getitem__")) {
			throw std::runtime_error("Dictionary malformed, keys(index 0) found within 'dict.values' is not a list");
		}
		if (!py::hasattr(values, "__getitem__")) {
			throw std::runtime_error("Dictionary malformed, values(index 1) found within 'dict.values' is not a list");
		}

		auto size = py::len(keys);
		if (size != py::len(values)) {
			throw std::runtime_error("Dictionary malformed, keys and values lists are not of the same size");
		}

		if (size == 0) {
			return Value::MAP(Value::EMPTYLIST(LogicalType::SQLNULL), Value::EMPTYLIST(LogicalType::SQLNULL));
		}

		auto key_values = TransformPythonValue(keys);
		auto val_values = TransformPythonValue(values);
		return Value::MAP(key_values, val_values);
	} else if (py::isinstance(ele, numpy_ndarray)) {
		return TransformPythonValue(ele.attr("tolist")());
	} else {
		throw std::runtime_error("TransformPythonValue unknown param type " + py::str(ele.get_type()).cast<string>());
	}
}

} // namespace duckdb
