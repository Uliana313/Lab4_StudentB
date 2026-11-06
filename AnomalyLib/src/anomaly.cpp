// anomaly.cpp - реалізація бібліотеки AnomalyLib
#include "anomaly.h"

#include <algorithm>
#include <cmath>
#include <new>
#include <vector>

namespace
{
    bool allFinite(const double* a, int n)
    {
        for (int i = 0; i < n; ++i)
            if (!std::isfinite(a[i]))
                return false;
        return true;
    }

    bool allEqual(const double* a, int n)
    {
        for (int i = 1; i < n; ++i)
            if (a[i] != a[0])
                return false;
        return true;
    }

    // Середнє та вибіркове стандартне відхилення (знаменник n-1).
    int computeStats(const double* data, int n, double& mean, double& sd)
    {
        if (allEqual(data, n))
            return ANOMALY_ERR_DEGENERATE;

        double sum = 0.0;
        for (int i = 0; i < n; ++i)
            sum += data[i];
        mean = sum / n;
        if (!std::isfinite(mean))
            return ANOMALY_ERR_OVERFLOW;

        double ss = 0.0;
        for (int i = 0; i < n; ++i)
        {
            const double d = data[i] - mean;
            ss += d * d;
        }
        if (!std::isfinite(ss))
            return ANOMALY_ERR_OVERFLOW;

        sd = std::sqrt(ss / (n - 1));
        if (!(sd > 0.0))
            return ANOMALY_ERR_DEGENERATE;   // значення різні, але розкид зник через underflow
        return ANOMALY_OK;
    }

    // Квантиль із лінійною інтерполяцією (як тип 7 у R / numpy.percentile). s - відсортований.
    double quantile(const std::vector<double>& s, double p)
    {
        const double pos = p * static_cast<double>(s.size() - 1);
        const size_t lo = static_cast<size_t>(std::floor(pos));
        const size_t hi = (lo + 1 < s.size()) ? lo + 1 : lo;
        const double frac = pos - static_cast<double>(lo);
        return s[lo] + frac * (s[hi] - s[lo]);
    }

    // flags може бути nullptr - тоді рахується лише кількість.
    int flagsByZ(const double* data, int n, double threshold, int* flags, int* count)
    {
        double mean = 0.0, sd = 0.0;
        int st = computeStats(data, n, mean, sd);
        if (st != ANOMALY_OK)
            return st;

        int c = 0;
        for (int i = 0; i < n; ++i)
        {
            const bool out = std::fabs((data[i] - mean) / sd) > threshold;
            if (flags != nullptr)
                flags[i] = out ? 1 : 0;
            c += out ? 1 : 0;
        }
        *count = c;
        return ANOMALY_OK;
    }

    int flagsByIqr(const double* data, int n, double k, int* flags, int* count,
                   double* lowerOut, double* upperOut)
    {
        std::vector<double> sorted(data, data + n);   // може кинути bad_alloc (ловиться вище)
        std::sort(sorted.begin(), sorted.end());

        const double q1 = quantile(sorted, 0.25);
        const double q3 = quantile(sorted, 0.75);
        const double iqr = q3 - q1;
        const double lower = q1 - k * iqr;
        const double upper = q3 + k * iqr;
        if (!std::isfinite(iqr) || !std::isfinite(lower) || !std::isfinite(upper))
            return ANOMALY_ERR_OVERFLOW;

        int c = 0;
        for (int i = 0; i < n; ++i)
        {
            const bool out = (data[i] < lower) || (data[i] > upper);
            if (flags != nullptr)
                flags[i] = out ? 1 : 0;
            c += out ? 1 : 0;
        }
        *count = c;
        if (lowerOut != nullptr) *lowerOut = lower;
        if (upperOut != nullptr) *upperOut = upper;
        return ANOMALY_OK;
    }

    // Спільна реалізація. Порядок перевірок: method -> n -> threshold -> значення.
    int detectImpl(const double* data, int n, int method, double threshold,
                   int* flags, int* count, double* lowerOut, double* upperOut)
    {
        if (method != ANOMALY_METHOD_ZSCORE && method != ANOMALY_METHOD_IQR)
            return ANOMALY_ERR_INVALID_PARAMETER;

        const int minCount = (method == ANOMALY_METHOD_ZSCORE) ? 2 : 4;
        if (n < minCount)
            return ANOMALY_ERR_INVALID_COUNT;

        if (!std::isfinite(threshold) || !(threshold > 0.0))
            return ANOMALY_ERR_INVALID_PARAMETER;

        if (!allFinite(data, n))
            return ANOMALY_ERR_INVALID_VALUE;

        try
        {
            if (method == ANOMALY_METHOD_ZSCORE)
                return flagsByZ(data, n, threshold, flags, count);
            return flagsByIqr(data, n, threshold, flags, count, lowerOut, upperOut);
        }
        catch (const std::bad_alloc&)
        {
            return ANOMALY_ERR_NO_MEMORY;
        }
        catch (...)
        {
            return ANOMALY_ERR_INTERNAL;
        }
    }
}

ANOMALY_API int zScore(const double* data, int n, double* out)
{
    if (data == nullptr || out == nullptr)
        return ANOMALY_ERR_NULL_POINTER;
    if (n < 2)
        return ANOMALY_ERR_INVALID_COUNT;
    if (!allFinite(data, n))
        return ANOMALY_ERR_INVALID_VALUE;

    double mean = 0.0, sd = 0.0;
    int st = computeStats(data, n, mean, sd);
    if (st != ANOMALY_OK)
        return st;

    // Перший прохід: перевіряємо, що всі z скінченні (out ще не чіпаємо).
    for (int i = 0; i < n; ++i)
        if (!std::isfinite((data[i] - mean) / sd))
            return ANOMALY_ERR_OVERFLOW;

    for (int i = 0; i < n; ++i)
        out[i] = (data[i] - mean) / sd;
    return ANOMALY_OK;
}

ANOMALY_API int iqrOutliers(const double* data, int n, double k, int* flags,
                            double* lowerBound, double* upperBound, int* outlierCount)
{
    if (data == nullptr || flags == nullptr || lowerBound == nullptr ||
        upperBound == nullptr || outlierCount == nullptr)
        return ANOMALY_ERR_NULL_POINTER;
    return detectImpl(data, n, ANOMALY_METHOD_IQR, k, flags, outlierCount, lowerBound, upperBound);
}

ANOMALY_API int detectOutliers(const double* data, int n, int method, double threshold,
                               int* flags, int* outlierCount)
{
    if (data == nullptr || flags == nullptr || outlierCount == nullptr)
        return ANOMALY_ERR_NULL_POINTER;
    return detectImpl(data, n, method, threshold, flags, outlierCount, nullptr, nullptr);
}

ANOMALY_API int anomalyRate(const double* data, int n, int method, double threshold, double* rate)
{
    if (data == nullptr || rate == nullptr)
        return ANOMALY_ERR_NULL_POINTER;

    int count = 0;
    int st = detectImpl(data, n, method, threshold, nullptr, &count, nullptr, nullptr);
    if (st != ANOMALY_OK)
        return st;

    *rate = static_cast<double>(count) / static_cast<double>(n);
    return ANOMALY_OK;
}

ANOMALY_API const char* anomalyVersion(void)
{
    return "1.0.0";
}

ANOMALY_API const char* anomalyErrorMessage(int status)
{
    switch (status)
    {
    case ANOMALY_OK:                    return "OK";
    case ANOMALY_ERR_NULL_POINTER:      return "Null pointer argument";
    case ANOMALY_ERR_INVALID_COUNT:     return "Invalid element count";
    case ANOMALY_ERR_INVALID_VALUE:     return "Input contains NaN or Inf";
    case ANOMALY_ERR_INVALID_PARAMETER: return "Invalid method or threshold";
    case ANOMALY_ERR_DEGENERATE:        return "Degenerate data (all values equal)";
    case ANOMALY_ERR_OVERFLOW:          return "Numeric overflow";
    case ANOMALY_ERR_NO_MEMORY:         return "Out of memory";
    case ANOMALY_ERR_INTERNAL:          return "Internal error";
    default:                            return "Unknown status code";
    }
}
