// Проєкт 2 (студент Б): НЕЯВНЕ зв'язування з RegressionLib партнера.
// Використовуються: regression.h (заголовок), RegressionLib.lib (імпортна бібліотека), RegressionLib.dll.
#include <iostream>
#include <limits>
#include <vector>

#include "regression.h"   // оголошення функцій з SDK партнера

static void show(const char* title, int status)
{
    std::cout << "  " << title << " -> status " << status << " (" << regressionErrorMessage(status) << ")\n";
}

int main()
{
    std::cout << "=== IMPLICIT linking: RegressionLib v" << regressionVersion() << " ===\n\n";

    // ---------- 1. Нормальний сценарій ----------
    std::cout << "1. Normal usage\n";
    std::vector<double> x = { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::vector<double> y = { 2.3, 4.1, 6.2, 7.9, 10.2, 11.8, 14.1, 16.0 };
    const int n = static_cast<int>(x.size());
    double slope = 0.0, intercept = 0.0, r2 = 0.0, prediction = 0.0;
    std::vector<double> res(x.size());

    int st = linearRegression(x.data(), y.data(), n, &slope, &intercept);
    show("linearRegression", st);
    if (st == REGRESSION_OK)
        std::cout << "     y = " << slope << " * x + " << intercept << "\n";

    st = rSquared(x.data(), y.data(), n, slope, intercept, &r2);
    show("rSquared", st);
    if (st == REGRESSION_OK)
        std::cout << "     R^2 = " << r2 << "\n";

    st = predict(slope, intercept, 10.0, &prediction);
    show("predict(x = 10)", st);
    if (st == REGRESSION_OK)
        std::cout << "     prediction = " << prediction << "\n";

    st = residuals(x.data(), y.data(), n, slope, intercept, res.data());
    show("residuals", st);
    if (st == REGRESSION_OK)
    {
        std::cout << "     residuals:";
        for (double e : res) std::cout << " " << e;
        std::cout << "\n";
    }

    // ---------- 2. Граничні та некоректні дані (тестування бібліотеки партнера) ----------
    std::cout << "\n2. Edge cases of partner's library\n";
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    double a = 0, b = 0, out3[3] = {};

    show("x == nullptr", linearRegression(nullptr, y.data(), n, &a, &b));
    show("y == nullptr", linearRegression(x.data(), nullptr, n, &a, &b));
    show("slope == nullptr", linearRegression(x.data(), y.data(), n, nullptr, &b));
    show("n == 0", linearRegression(x.data(), y.data(), 0, &a, &b));
    show("n == 1", linearRegression(x.data(), y.data(), 1, &a, &b));
    show("n == -5", linearRegression(x.data(), y.data(), -5, &a, &b));

    double xnan[3] = { 1, nan, 3 };
    double yinf[3] = { 1, inf, 3 };
    double yok[3] = { 1, 2, 3 };
    show("NaN in x", linearRegression(xnan, yok, 3, &a, &b));
    show("Inf in y", linearRegression(yok, yinf, 3, &a, &b));

    double xsame[3] = { 5, 5, 5 };
    show("all x equal (vertical line)", linearRegression(xsame, yok, 3, &a, &b));

    double xo[2] = { 0, 1 };
    double yo[2] = { -1.7e308, 1.7e308 };
    show("values ~1e308 (overflow)", linearRegression(xo, yo, 2, &a, &b));

    double yconst[3] = { 4, 4, 4 };
    show("rSquared: all y equal", rSquared(yok, yconst, 3, 0.0, 4.0, &r2));
    show("rSquared: slope == NaN", rSquared(x.data(), y.data(), n, nan, 0.0, &r2));
    show("predict overflow", predict(1e308, 0.0, 10.0, &prediction));
    show("predict: NaN", predict(nan, 0.0, 1.0, &prediction));
    show("residuals: out == nullptr", residuals(x.data(), y.data(), n, 1.0, 0.0, nullptr));
    show("residuals: n == 3 (valid)", residuals(yok, yok, 3, 1.0, 0.0, out3));

    // ---------- 3. Надвеликий масив ----------
    std::cout << "\n3. Large array (5 000 000 points, y = 2x + 1)\n";
    const int N = 5000000;
    std::vector<double> bx(N), by(N);
    for (int i = 0; i < N; ++i)
    {
        bx[i] = i;
        by[i] = 2.0 * i + 1.0;
    }
    st = linearRegression(bx.data(), by.data(), N, &a, &b);
    show("linearRegression on 5M points", st);
    if (st == REGRESSION_OK)
        std::cout << "     slope = " << a << ", intercept = " << b << "\n";

    std::cout << "\nDone. The program finished without crashing.\n";
    return 0;
}
