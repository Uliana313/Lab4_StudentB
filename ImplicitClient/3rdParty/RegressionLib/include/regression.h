// regression.h - публічний інтерфейс бібліотеки RegressionLib (версія API 1.0)
// Регресійний аналіз: лінійна регресія методом найменших квадратів.
//
// Інтерфейс має C-ABI (extern "C"): імена функцій не декоруються,
// тому їх можна знайти через GetProcAddress() за простим ім'ям.
#pragma once

#ifdef __cplusplus
  #define REGRESSION_C extern "C"
#else
  #define REGRESSION_C
#endif

// Макрос REGRESSION_API:
//  * при збірці самої DLL (визначено REGRESSION_EXPORTS) -> dllexport;
//  * при використанні DLL клієнтом                       -> dllimport;
//  * при збірці як статична бібліотека (REGRESSION_STATIC) -> нічого.
#if defined(_WIN32) && !defined(REGRESSION_STATIC)
  #ifdef REGRESSION_EXPORTS
    #define REGRESSION_API REGRESSION_C __declspec(dllexport)
  #else
    #define REGRESSION_API REGRESSION_C __declspec(dllimport)
  #endif
#else
  #define REGRESSION_API REGRESSION_C
#endif

#define REGRESSION_API_VERSION_MAJOR 1
#define REGRESSION_API_VERSION_MINOR 0

// Коди повернення. Усі функції повертають int зі значенням з цього переліку.
enum RegressionStatus
{
    REGRESSION_OK                 = 0,  // успіх
    REGRESSION_ERR_NULL_POINTER   = 1,  // один з обов'язкових вказівників дорівнює nullptr
    REGRESSION_ERR_INVALID_COUNT  = 2,  // кількість елементів n недопустима
    REGRESSION_ERR_INVALID_VALUE  = 3,  // у вхідних даних є NaN або +-Inf
    REGRESSION_ERR_DEGENERATE     = 4,  // задача не має однозначного розв'язку
    REGRESSION_ERR_OVERFLOW       = 5   // результат не вміщується у double (inf/NaN)
};

// Лінійна регресія y = slope * x + intercept (метод найменших квадратів).
REGRESSION_API int linearRegression(const double* x, const double* y, int n,
                                    double* slope, double* intercept);

// Прогноз y = slope * x + intercept для одного значення x.
REGRESSION_API int predict(double slope, double intercept, double x, double* result);

// Коефіцієнт детермінації R^2 для заданої прямої.
REGRESSION_API int rSquared(const double* x, const double* y, int n,
                            double slope, double intercept, double* r2);

// Залишки e[i] = y[i] - (slope * x[i] + intercept). Масив out виділяє викликач (n елементів).
REGRESSION_API int residuals(const double* x, const double* y, int n,
                             double slope, double intercept, double* out);

// Службові функції. Повертають вказівник на статичний рядок; звільняти його НЕ можна.
REGRESSION_API const char* regressionVersion(void);
REGRESSION_API const char* regressionErrorMessage(int status);
