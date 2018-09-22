#pragma once

#include <boost/python/numpy.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>

#include "ruleextraction/RuleExtractor.hpp"

namespace hx {
namespace python {
namespace py = boost::python;
namespace np = boost::python::numpy;

class RuleExtractor {
public:
    RuleExtractor(std::size_t populationSize,
                  np::ndarray controlPoints,
                  np::ndarray rulePoints,
                  np::ndarray controlClasses,
                  np::ndarray ruleClasses,
                  double crossoverFactor = 0.33,
                  double mutationFactor = 0.1) {
        std::vector<std::size_t> ruleClassesValues;
        std::vector<std::size_t> controlClassesValues;

        auto expectedTypeClass = np::dtype::get_builtin<std::size_t>();
        auto expectedTypePoints = np::dtype::get_builtin<float>();
        if (!np::equivalent(controlClasses.get_dtype(), expectedTypeClass)
            || !np::equivalent(ruleClasses.get_dtype(), expectedTypeClass))
            throw std::invalid_argument(
                "Cannot use class array of this type, cast to uint64 before creating "
                "object");

        if (!np::equivalent(rulePoints.get_dtype(), expectedTypePoints)
            || !np::equivalent(controlPoints.get_dtype(), expectedTypePoints))
            throw std::invalid_argument("Cannot use extractor on non-float datatype");

        auto sizeofRuleClasses = 1;
        auto sizeofControlClasses = 1;
        std::size_t dims = rulePoints.shape(-1);

        for (auto i = 0; i < ruleClasses.get_nd(); ++i) {
            sizeofRuleClasses *= ruleClasses.shape(i);
        }

        for (auto i = 0; i < controlClasses.get_nd(); ++i) {
            sizeofControlClasses *= controlClasses.shape(i);
        }

        for (auto ptr = reinterpret_cast<std::size_t*>(ruleClasses.get_data());
             sizeofRuleClasses > 0;
             --sizeofRuleClasses, ++ptr) {
            ruleClassesValues.push_back(*ptr);
        }

        for (auto ptr = reinterpret_cast<std::size_t*>(controlClasses.get_data());
             sizeofControlClasses > 0;
             --sizeofControlClasses, ++ptr) {
            controlClassesValues.push_back(*ptr);
        }

        _pImpl = std::make_unique<hx::ruleextraction::RuleExtractor>(
            populationSize,
            dims,
            std::move(controlClassesValues),
            std::move(ruleClassesValues),
            crossoverFactor,
            mutationFactor);

        auto controlData = _pImpl->getControlCentres();
        auto rulesData = _pImpl->getRulesCentres();
        auto dim = _pImpl->getDims();

        auto numpyDataRules = reinterpret_cast<float*>(rulePoints.get_data());
        if (_pImpl->getRulesNumber() != (std::size_t) rulePoints.shape(0)
            || dim != (std::size_t) rulePoints.shape(1))
            std::cerr << "Size rules mismatches\n";

        for (std::size_t i = 0; i < _pImpl->getRulesNumber(); ++i) {
            std::copy(numpyDataRules, numpyDataRules + _pImpl->getDims(), rulesData);
            rulesData += _pImpl->getStride();
            numpyDataRules += rulePoints.strides(0) / sizeof(float);
        }

        auto numpyDataControl = reinterpret_cast<float*>(controlPoints.get_data());

        for (std::size_t i = 0; i < _pImpl->getControlPointsNumber(); ++i) {
            std::copy(
                numpyDataControl, numpyDataControl + _pImpl->getDims(), controlData);
            controlData += _pImpl->getStride();
            numpyDataControl += controlPoints.strides(0) / sizeof(float);
        }
    }

    std::size_t fitEpoch() { return _pImpl->fitEpoch(); }

    np::ndarray getBest() const {
        np::ndarray result =
            np::zeros(py::make_tuple(_pImpl->getRulesNumber(), _pImpl->getDims() * 2 + 1),
                      np::dtype::get_builtin<float>());

        auto resultData = reinterpret_cast<float*>(result.get_data());
        auto rulesCentres = _pImpl->getRulesCentres();
        auto rulesData = _pImpl->getBest();
        auto rulesClasses = _pImpl->ruleClasses();

        // std::cerr << rulesData.getScore() << std::endl;

        for (std::size_t i = 0; i < _pImpl->getRulesNumber(); ++i) {
            for (std::size_t j = 0; j < _pImpl->getDims(); ++j) {
                resultData[2 * j] = rulesCentres[j] - rulesData[j];
                resultData[2 * j + 1] =
                    rulesCentres[j] + rulesData[j + _pImpl->getStride()];
            }
            resultData[_pImpl->getDims() * 2] = *rulesClasses;
            resultData += _pImpl->getDims() * 2 + 1;
            rulesCentres += _pImpl->getStride();
            rulesData += _pImpl->getStride() * 2;
            rulesClasses++;
        }

        return result;
    }

private:
    std::unique_ptr<hx::ruleextraction::RuleExtractor> _pImpl;
};
}// namespace python
}// namespace hx