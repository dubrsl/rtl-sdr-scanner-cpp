#include "utils.h"

#include <liquid/liquid.h>
#include <logger.h>
#include <math.h>
#include <unistd.h>

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <thread>

uint32_t getThreadId() {
  return gettid();
}

uint32_t getSamplesCount(const Frequency &sampleRate, const std::chrono::milliseconds &time) {
  if (time.count() >= 1000) {
    if (time.count() * sampleRate % 1000 != 0) {
      throw std::runtime_error("selected time not fit to sample rate");
    }
    return 2 * time.count() * sampleRate / 1000;
  } else {
    const auto samplesCount = std::lround(sampleRate / (1000.0f / time.count()) * 2);
    if (samplesCount % 512 != 0) {
      Logger::warn("utils", "samples count {} not fit 512", samplesCount);
      throw std::runtime_error("selected time not fit to sample rate");
    }
    return samplesCount;
  }
}

void toComplex(const uint8_t *rawBuffer, std::vector<std::complex<float>> &buffer, uint32_t samplesCount) {
  for (uint32_t i = 0; i < samplesCount; ++i) {
    buffer[i] = std::complex<float>((rawBuffer[2 * i] - 127.5f) / 127.5f, (rawBuffer[2 * i + 1] - 127.5f) / 127.5f);
  }
}

std::chrono::milliseconds time() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()); }

std::vector<std::complex<float>> getShiftData(int32_t frequencyOffset, Frequency sampleRate, uint32_t samplesCount) {
  const auto f = std::complex<float>(0.0f, -1.0f) * 2.0f * M_PIf32 * (static_cast<float>(-frequencyOffset) / static_cast<float>(sampleRate));
  std::vector<std::complex<float>> data(samplesCount);
  for (uint32_t i = 0; i < samplesCount; ++i) {
    data[i] = std::exp(f * static_cast<float>(i));
  }
  return data;
}

void shift(std::vector<std::complex<float>> &samples, const std::vector<std::complex<float>> &factors, uint32_t samplesCount) {
  for (uint32_t i = 0; i < samplesCount; ++i) {
    samples[i] *= factors[i];
  }
}

liquid_float_complex *toLiquidComplex(std::complex<float> *ptr) { return reinterpret_cast<liquid_float_complex *>(ptr); }

std::vector<FrequencyRange> fitFrequencyRange(const UserDefinedFrequencyRange &userRange) {
  const auto range = userRange.stop - userRange.start;
  if (userRange.sampleRate < range) {
    const auto cutDigits = floor(log10(userRange.sampleRate));
    const auto cutBase = pow(10.0f, cutDigits);
    const auto firstDigits = floor(userRange.sampleRate / cutBase);
    Frequency subSampleRate = firstDigits * cutBase;
    if (range <= subSampleRate) {
      subSampleRate = range / 2;
    }
    std::vector<FrequencyRange> results;
    for (Frequency i = userRange.start; i < userRange.stop; i += subSampleRate) {
      for (const auto &range : fitFrequencyRange({i, i + subSampleRate, userRange.step, userRange.sampleRate})) {
        results.push_back(range);
      }
    }
    return results;
  }
  const auto fftSize = userRange.sampleRate / userRange.step;
  if (fftSize != pow(2.0f, ceil(log2(fftSize)))) {
    Logger::warn("utils", "range {}, step and sample rate not fit, calculated fft size: {}", userRange.toString(), fftSize);
    throw std::runtime_error("");
  }
  return {{userRange.start, userRange.stop, userRange.step, userRange.sampleRate}};
}
