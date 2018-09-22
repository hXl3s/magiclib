#include <boost/python/numpy.hpp>

#include "python/RuleExtractorPython.hpp"

namespace py = boost::python;
namespace np = boost::python::numpy;

char const* greet() {
    return "hello, world";
}

BOOST_PYTHON_MODULE(libhxtk) {
    np::initialize();

    py::class_<hx::python::RuleExtractor, boost::noncopyable>("RuleExtractor",
                                                              py::init<std::size_t,
                                                                       np::ndarray,
                                                                       np::ndarray,
                                                                       np::ndarray,
                                                                       np::ndarray,
                                                                       double,
                                                                       double>())
        .def("fitEpoch", &hx::python::RuleExtractor::fitEpoch)
        .def("getResult", &hx::python::RuleExtractor::getBest);
}