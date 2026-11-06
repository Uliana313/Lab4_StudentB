// Локальний тест AnomalyLib (етап 1 - статична бібліотека, етап 2 - перевірка DLL)
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "anomaly.h"

static int g_failed = 0;

static void check(bool ok, const char* what)
{
    std::cout << (ok ? "  [PASS] " : "  [FAIL] ") << what << "\n";
    if (!ok)
        ++g_failed;
}

static bool approx(double a, double b, double eps = 1e-9)
{
    return std::fabs(a - b) <= eps;
}

int main()
{
    std::cout << "AnomalyLib version " << anomalyVersion() << "\n\n";

    // ---- 1. Типові дані: 12 "нормальних" значень і одне явне відхилення (100) ----
    std::cout << "1. Typical data (13 values, outlier at index 12)\n";
    const double data[13] = { 10, 12, 11, 13, 12, 11, 10, 12, 13, 11, 12, 11, 100 };
    const int n = 13;
    double z[13] = {};
    int flags[13] = {};
    int count = -1;
    double lower = 0, upper = 0, rate = -1;

    check(zScore(data, n, z) == ANOMALY_OK, "zScore returns OK");
    std::cout << "     z[12] = " << z[12] << "\n";
    check(approx(z[12], 3.3257, 1e-3), "z[12] ~ 3.3257 (reference from NumPy)");
    check(approx(z[0], -0.3382, 1e-3), "z[0] ~ -0.3382");

    check(detectOutliers(data, n, ANOMALY_METHOD_ZSCORE, 3.0, flags, &count) == ANOMALY_OK, "detectOutliers(z-score) OK");
    check(count == 1 && flags[12] == 1 && flags[0] == 0, "only index 12 flagged (z-score, threshold 3)");

    check(iqrOutliers(data, n, 1.5, flags, &lower, &upper, &count) == ANOMALY_OK, "iqrOutliers returns OK");
    std::cout << "     bounds = [" << lower << ", " << upper << "]\n";
    check(approx(lower, 9.5) && approx(upper, 13.5), "bounds == [9.5, 13.5]");
    check(count == 1 && flags[12] == 1, "only index 12 flagged (IQR, k = 1.5)");

    check(anomalyRate(data, n, ANOMALY_METHOD_IQR, 1.5, &rate) == ANOMALY_OK, "anomalyRate OK");
    check(approx(rate, 1.0 / 13.0), "rate == 1/13");

    // ---- 2. Дані без викидів ----
    std::cout << "\n2. Data without outliers\n";
    const double clean[6] = { 5, 6, 5, 6, 5, 6 };
    int flagsClean[6] = {};
    check(detectOutliers(clean, 6, ANOMALY_METHOD_IQR, 1.5, flagsClean, &count) == ANOMALY_OK && count == 0, "count == 0");
    check(anomalyRate(clean, 6, ANOMALY_METHOD_ZSCORE, 3.0, &rate) == ANOMALY_OK && rate == 0.0, "rate == 0");

    // ---- 3. Граничні та некоректні дані ----
    std::cout << "\n3. Edge cases\n";
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    count = -777; rate = -777;
    check(zScore(nullptr, n, z) == ANOMALY_ERR_NULL_POINTER, "zScore: data == nullptr");
    check(zScore(data, n, nullptr) == ANOMALY_ERR_NULL_POINTER, "zScore: out == nullptr");
    check(zScore(data, 0, z) == ANOMALY_ERR_INVALID_COUNT, "zScore: n == 0");
    check(zScore(data, 1, z) == ANOMALY_ERR_INVALID_COUNT, "zScore: n == 1");
    check(zScore(data, -3, z) == ANOMALY_ERR_INVALID_COUNT, "zScore: n < 0");

    const double same[5] = { 7, 7, 7, 7, 7 };
    double zs[5] = {};
    check(zScore(same, 5, zs) == ANOMALY_ERR_DEGENERATE, "zScore: all values equal");
    const double same2[4] = { 0.1, 0.1, 0.1, 0.1 };
    double zs2[4] = {};
    check(zScore(same2, 4, zs2) == ANOMALY_ERR_DEGENERATE, "zScore: all values 0.1 (rounding safe)");

    const double withNan[4] = { 1, nan, 3, 4 };
    const double withInf[4] = { 1, 2, inf, 4 };
    int f4[4] = {};
    check(detectOutliers(withNan, 4, ANOMALY_METHOD_ZSCORE, 3.0, f4, &count) == ANOMALY_ERR_INVALID_VALUE, "NaN in data");
    check(detectOutliers(withInf, 4, ANOMALY_METHOD_IQR, 1.5, f4, &count) == ANOMALY_ERR_INVALID_VALUE, "Inf in data");

    check(detectOutliers(data, n, 99, 3.0, flags, &count) == ANOMALY_ERR_INVALID_PARAMETER, "unknown method");
    check(detectOutliers(data, n, ANOMALY_METHOD_ZSCORE, 0.0, flags, &count) == ANOMALY_ERR_INVALID_PARAMETER, "threshold == 0");
    check(detectOutliers(data, n, ANOMALY_METHOD_ZSCORE, -1.0, flags, &count) == ANOMALY_ERR_INVALID_PARAMETER, "threshold < 0");
    check(detectOutliers(data, n, ANOMALY_METHOD_ZSCORE, nan, flags, &count) == ANOMALY_ERR_INVALID_PARAMETER, "threshold == NaN");
    check(detectOutliers(data, 3, ANOMALY_METHOD_IQR, 1.5, flags, &count) == ANOMALY_ERR_INVALID_COUNT, "IQR requires n >= 4");
    check(detectOutliers(nullptr, n, ANOMALY_METHOD_ZSCORE, 3.0, flags, &count) == ANOMALY_ERR_NULL_POINTER, "detect: data == nullptr");
    check(detectOutliers(data, n, ANOMALY_METHOD_ZSCORE, 3.0, nullptr, &count) == ANOMALY_ERR_NULL_POINTER, "detect: flags == nullptr");
    check(detectOutliers(data, n, ANOMALY_METHOD_ZSCORE, 3.0, flags, nullptr) == ANOMALY_ERR_NULL_POINTER, "detect: outlierCount == nullptr");
    check(iqrOutliers(data, n, 1.5, flags, nullptr, &upper, &count) == ANOMALY_ERR_NULL_POINTER, "iqr: lowerBound == nullptr");
    check(anomalyRate(data, n, ANOMALY_METHOD_ZSCORE, 3.0, nullptr) == ANOMALY_ERR_NULL_POINTER, "rate: rate == nullptr");

    const double huge[4] = { -1.7e308, 1.7e308, 1.7e308, -1.7e308 };
    check(detectOutliers(huge, 4, ANOMALY_METHOD_ZSCORE, 3.0, f4, &count) == ANOMALY_ERR_OVERFLOW, "z-score overflow (1e308)");

    check(count == -777 && rate == -777, "outputs untouched after errors");

    // ---- 4. Великий масив ----
    std::cout << "\n4. Large array (5 000 000 values, one outlier)\n";
    const int N = 5000000;
    std::vector<double> big(N);
    std::vector<int> bigFlags(N);
    for (int i = 0; i < N; ++i)
        big[i] = static_cast<double>(i % 100);
    big[123456] = 1.0e6;

    check(detectOutliers(big.data(), N, ANOMALY_METHOD_ZSCORE, 3.0, bigFlags.data(), &count) == ANOMALY_OK, "z-score OK");
    check(count == 1 && bigFlags[123456] == 1, "exactly the planted outlier is found (z-score)");
    check(detectOutliers(big.data(), N, ANOMALY_METHOD_IQR, 1.5, bigFlags.data(), &count) == ANOMALY_OK, "IQR OK");
    check(count == 1 && bigFlags[123456] == 1, "exactly the planted outlier is found (IQR)");

    std::cout << "\n" << (g_failed == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << "\n";
    return g_failed == 0 ? 0 : 1;
}
