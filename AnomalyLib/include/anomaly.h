// anomaly.h - публічний інтерфейс бібліотеки AnomalyLib (версія API 1.0)
// Виявлення аномалій (викидів) у числових даних: метод z-score та метод IQR.
//
// Інтерфейс має C-ABI (extern "C"): імена функцій не декоруються,
// тому їх можна знайти через GetProcAddress() за простим ім'ям.
#pragma once

#ifdef __cplusplus
  #define ANOMALY_C extern "C"
#else
  #define ANOMALY_C
#endif

// ANOMALY_EXPORTS - лише при збірці самої DLL; ANOMALY_STATIC - статична збірка.
#if defined(_WIN32) && !defined(ANOMALY_STATIC)
  #ifdef ANOMALY_EXPORTS
    #define ANOMALY_API ANOMALY_C __declspec(dllexport)
  #else
    #define ANOMALY_API ANOMALY_C __declspec(dllimport)
  #endif
#else
  #define ANOMALY_API ANOMALY_C
#endif

#define ANOMALY_API_VERSION_MAJOR 1
#define ANOMALY_API_VERSION_MINOR 0

// Коди повернення. Усі функції повертають int зі значенням з цього переліку.
enum AnomalyStatus
{
    ANOMALY_OK                    = 0,  // успіх
    ANOMALY_ERR_NULL_POINTER      = 1,  // обов'язковий вказівник дорівнює nullptr
    ANOMALY_ERR_INVALID_COUNT     = 2,  // кількість елементів n недопустима
    ANOMALY_ERR_INVALID_VALUE     = 3,  // у вхідних даних є NaN або +-Inf
    ANOMALY_ERR_INVALID_PARAMETER = 4,  // недопустимий method або threshold
    ANOMALY_ERR_DEGENERATE        = 5,  // усі значення однакові (розкид = 0)
    ANOMALY_ERR_OVERFLOW          = 6,  // проміжний результат не вміщується у double
    ANOMALY_ERR_NO_MEMORY         = 7,  // не вистачило пам'яті всередині бібліотеки
    ANOMALY_ERR_INTERNAL          = 8   // непередбачена внутрішня помилка
};

// Метод виявлення для detectOutliers() та anomalyRate().
enum AnomalyMethod
{
    ANOMALY_METHOD_ZSCORE = 0,  // |z| > threshold (типово 3.0)
    ANOMALY_METHOD_IQR    = 1   // поза [Q1 - k*IQR, Q3 + k*IQR] (типово k = 1.5)
};

// z-оцінки всіх елементів: out[i] = (data[i] - mean) / s, s - вибіркове станд. відхилення (n-1).
ANOMALY_API int zScore(const double* data, int n, double* out);

// Викиди за методом IQR. flags[i] = 1 для викиду, 0 для звичайного значення.
ANOMALY_API int iqrOutliers(const double* data, int n, double k, int* flags,
                            double* lowerBound, double* upperBound, int* outlierCount);

// Універсальне виявлення викидів обраним методом.
ANOMALY_API int detectOutliers(const double* data, int n, int method, double threshold,
                               int* flags, int* outlierCount);

// Частка аномалій rate = (кількість викидів) / n, у діапазоні [0, 1].
ANOMALY_API int anomalyRate(const double* data, int n, int method, double threshold, double* rate);

// Службові функції. Повертають вказівник на статичний рядок; звільняти його НЕ можна.
ANOMALY_API const char* anomalyVersion(void);
ANOMALY_API const char* anomalyErrorMessage(int status);
