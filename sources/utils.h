#pragma once

#include <config.h>
#include <liquid/liquid.h>
#include <radio/help_structures.h>

#include <chrono>
#include <complex>
#include <optional>
#include <vector>

uint32_t getThreadId();

uint32_t getSamplesCount(const Frequency& sampleRate, const std::chrono::milliseconds& time);

void toComplex(const uint8_t* rawBuffer, std::vector<std::complex<float>>& buffer, uint32_t samplesCount);

std::chrono::milliseconds time();

std::vector<std::complex<float>> getShiftData(int32_t frequencyOffset, Frequency sampleRate, uint32_t samplesCount);

void shift(std::vector<std::complex<float>>& samples, const std::vector<std::complex<float>>& factors, uint32_t samplesCount);

liquid_float_complex* toLiquidComplex(std::complex<float>* ptr);

std::vector<FrequencyRange> fitFrequencyRange(const UserDefinedFrequencyRange& userRange);
