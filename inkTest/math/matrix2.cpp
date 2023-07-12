#include <catch_amalgamated.hpp>
#include <ink/math/matrix.h>

using namespace ink;

static auto near(float a, float b, float eps = FLT_EPSILON) noexcept -> bool {
    return std::abs(a - b) <= eps;
}

static auto near(Vector2 a, Vector2 b, float eps = FLT_EPSILON) noexcept -> bool {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps);
}

static auto near(const Matrix2 &a, const Matrix2 &b, float eps = FLT_EPSILON) noexcept -> bool {
    return near(a[0], b[0], eps) && near(a[1], b[1], eps);
}

TEST_CASE("Matrix2 construct", "[Matrix2]") {
    // [0, 0]
    // [0, 0]
    Matrix2 zero;
    REQUIRE(zero[0][0] == 0);
    REQUIRE(zero[0][1] == 0);
    REQUIRE(zero[1][0] == 0);
    REQUIRE(zero[1][1] == 0);

    // [1, 0]
    // [0, 1]
    Matrix2 identity(1.0f);
    REQUIRE(identity[0][0] == 1.0f);
    REQUIRE(identity[0][1] == 0.0f);
    REQUIRE(identity[1][0] == 0.0f);
    REQUIRE(identity[1][1] == 1.0f);

    // [1, 0]
    // [0, 2]
    Matrix2 a(1.0f, 2.0f);
    REQUIRE(a[0][0] == 1.0f);
    REQUIRE(a[0][1] == 0.0f);
    REQUIRE(a[1][0] == 0.0f);
    REQUIRE(a[1][1] == 2.0f);

    // [1, 3]
    // [2, 4]
    Matrix2 b(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(b[0][0] == 1.0f);
    REQUIRE(b[0][1] == 2.0f);
    REQUIRE(b[1][0] == 3.0f);
    REQUIRE(b[1][1] == 4.0f);

    Matrix2 c(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
    REQUIRE(b == c);
}

TEST_CASE("Matrix2 determinant", "[Matrix2]") {
    // [0, 0]
    // [0, 0]
    Matrix2 zero;
    REQUIRE(zero.determinant() == 0);

    // [1, 0]
    // [0, 1]
    Matrix2 identity(1.0f);
    REQUIRE(identity.determinant() == 1.0f);

    // [1, 0]
    // [0, 2]
    Matrix2 a(1.0f, 2.0f);
    REQUIRE(a.determinant() == 2.0f);

    // [1, 3]
    // [2, 4]
    Matrix2 b(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(b.determinant() == -2.0f);
}

TEST_CASE("Matrix2 transpose", "[Matrix2]") {
    // [0, 0]
    // [0, 0]
    Matrix2 zero;
    Matrix2 zeroTransposed = zero.transposed();
    zero.transpose();
    REQUIRE(zero == Matrix2());
    REQUIRE(zero == zeroTransposed);

    // [1, 0]
    // [0, 1]
    Matrix2 identity(1.0f);
    Matrix2 identityTransposed = identity.transposed();
    identity.transpose();
    REQUIRE(identity == Matrix2(1.0f));
    REQUIRE(identity == identityTransposed);

    // [1, 3]
    // [2, 4]
    Matrix2 a(1.0f, 2.0f, 3.0f, 4.0f);
    Matrix2 aTransposed = a.transposed();
    a.transpose();
    REQUIRE(a == Matrix2(1.0f, 3.0f, 2.0f, 4.0f));
    REQUIRE(a == aTransposed);
}

TEST_CASE("Matrix2 inverse", "[Matrix2]") {
    // [1, 0]
    // [0, 1]
    Matrix2 identity(1.0f);
    Matrix2 identityInversed = identity.inversed();
    REQUIRE(identity == identityInversed);
    REQUIRE((identity * identityInversed) == Matrix2(1.0f));

    // [1, 3]
    // [2, 4]
    Matrix2 a(1.0f, 2.0f, 3.0f, 4.0f);
    Matrix2 aInversed = a.inversed();
    REQUIRE(a != aInversed);
    REQUIRE(near(a * aInversed, Matrix2(1.0f)));

    a.inverse();
    REQUIRE(a == aInversed);
}

TEST_CASE("Matrix2 add operator", "[Matrix2]") {
    SECTION("Add scalars") {
        Matrix2 zero;
        Matrix2 a = zero + 1.0f;
        Matrix2 b = 1.0f + zero;
        REQUIRE(a == b);
        REQUIRE(a[0][0] == 1.0f);
        REQUIRE(a[0][1] == 1.0f);
        REQUIRE(a[1][0] == 1.0f);
        REQUIRE(a[1][1] == 1.0f);

        // [1, 3]
        // [2, 4]
        Matrix2 c(1.0f, 2.0f, 3.0f, 4.0f);

        // [2, 4]
        // [3, 5]
        Matrix2 d = c + 1.0f;

        c += 1.0f;
        REQUIRE(c == d);
        REQUIRE(c[0][0] == 2.0f);
        REQUIRE(c[0][1] == 3.0f);
        REQUIRE(c[1][0] == 4.0f);
        REQUIRE(c[1][1] == 5.0f);
    }

    SECTION("Add matrices") {
        Matrix2 a(1.0f, 2.0f, 3.0f, 4.0f);
        Matrix2 b(4.0f, 3.0f, 2.0f, 1.0f);
        Matrix2 c = a + b;
        a += b;
        REQUIRE(a == c);
        REQUIRE(a == (Matrix2() + 5.0f));
    }
}

TEST_CASE("Matrix2 subtract", "[Matrix2]") {
    SECTION("Subtract scalars") {
        Matrix2 zero;
        Matrix2 a = zero - 1.0f;
        Matrix2 b = 1.0f - zero;
        REQUIRE(a != b);
        REQUIRE(a == -b);
        REQUIRE(a[0][0] == -1.0f);
        REQUIRE(a[0][1] == -1.0f);
        REQUIRE(a[1][0] == -1.0f);
        REQUIRE(a[1][1] == -1.0f);

        // [1, 3]
        // [2, 4]
        Matrix2 c(1.0f, 2.0f, 3.0f, 4.0f);

        // [0, 2]
        // [1, 3]
        Matrix2 d = c - 1.0f;

        c -= 1.0f;
        REQUIRE(c == d);
        REQUIRE(c[0][0] == 0.0f);
        REQUIRE(c[0][1] == 1.0f);
        REQUIRE(c[1][0] == 2.0f);
        REQUIRE(c[1][1] == 3.0f);
    }

    SECTION("Subtract matrices") {
        Matrix2 a(1.0f, 2.0f, 3.0f, 4.0f);
        Matrix2 b(4.0f, 3.0f, 2.0f, 1.0f);
        Matrix2 c = a - b;
        a -= b;
        REQUIRE(a == c);
        REQUIRE(a[0][0] == -3.0f);
        REQUIRE(a[0][1] == -1.0f);
        REQUIRE(a[1][0] == 1.0f);
        REQUIRE(a[1][1] == 3.0f);
    }
}

TEST_CASE("Matrix2 multiply", "[Matrix2]") {
    SECTION("Multiply scalars") {
        // [1, 0]
        // [0, 1]
        Matrix2 a(1.0f);
        Matrix2 b = 2.0f * a;
        Matrix2 c = a * 2.0f;
        a *= 2.0f;
        REQUIRE(a == b);
        REQUIRE(a == c);
        REQUIRE(a == Matrix2(2.0f));
    }

    SECTION("Multiply vectors") {
        // [1, 3]
        // [2, 4]
        Matrix2 a(1.0f, 2.0f, 3.0f, 4.0f);
        Vector2 b(1.0f, 2.0f);

        // [5, 11]
        Vector2 c = b * a;
        REQUIRE(c == Vector2(5.0f, 11.0f));

        // [7, 10]
        Vector2 d = a * b;
        REQUIRE(d == Vector2(7.0f, 10.0f));

        b *= a;
        REQUIRE(b == c);
    }

    SECTION("Multiply matrices") {
        Matrix2 zero;
        Matrix2 identity(1.0f);

        // [1, 3]
        // [2, 4]
        Matrix2 a(1.0f, 2.0f, 3.0f, 4.0f);

        REQUIRE((a * zero) == zero);
        REQUIRE((zero * a) == zero);

        REQUIRE((a * identity) == a);
        REQUIRE((identity * a) == a);

        // [2, 3]
        // [1, 4]
        Matrix2 b(2.0f, 1.0f, 3.0f, 4.0f);

        // [5, 15]
        // [8, 22]
        Matrix2 c = a * b;
        REQUIRE(c == Matrix2(5.0f, 8.0f, 15.0f, 22.0f));

        // [8, 18]
        // [9, 19]
        Matrix2 d = b * a;
        REQUIRE(d == Matrix2(8.0f, 9.0f, 18.0f, 19.0f));
    }
}

TEST_CASE("Matrix2 divide", "[Matrix2]") {
    SECTION("Divide by scalars") {
        Matrix2 identity(1.0f);
        Matrix2 a = identity / 2.0f;
        REQUIRE(a == Matrix2(0.5f));

        Matrix2 b(2.0f, 2.0f, 2.0f, 2.0f);
        Matrix2 c = 1.0f / b;
        REQUIRE(c == Matrix2(0.5f, 0.5f, 0.5f, 0.5f));
    }

    SECTION("Divide by matrices") {
        Matrix2 identity(1.0f);
        Matrix2 a(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE((a / a) == identity);
        REQUIRE((identity / a) == a.inversed());
    }
}
