#include <catch_amalgamated.hpp>
#include <ink/math/matrix.h>
#include <ink/math/number.h>

using namespace ink;

static auto near(float a, float b, float eps = FLT_EPSILON) noexcept -> bool {
    return std::abs(a - b) <= eps;
}

static auto near(Vector3 a, Vector3 b, float eps = FLT_EPSILON) noexcept -> bool {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps) && near(a.z, b.z, eps);
}

static auto near(const Matrix3 &a, const Matrix3 &b, float eps = FLT_EPSILON) noexcept -> bool {
    return near(a[0], b[0], eps) && near(a[1], b[1], eps) && near(a[2], b[2], eps);
}

TEST_CASE("Matrix3 construct", "[Matrix3]") {
    // [0, 0, 0]
    // [0, 0, 0]
    // [0, 0, 0]
    Matrix3 zero;
    REQUIRE(zero[0] == Vector3());
    REQUIRE(zero[1] == Vector3());
    REQUIRE(zero[2] == Vector3());

    // [1, 0, 0]
    // [0, 1, 0]
    // [0, 0, 1]
    Matrix3 identity(1.0f);
    REQUIRE(identity[0][0] == 1.0f);
    REQUIRE(identity[0][1] == 0.0f);
    REQUIRE(identity[0][2] == 0.0f);
    REQUIRE(identity[1][0] == 0.0f);
    REQUIRE(identity[1][1] == 1.0f);
    REQUIRE(identity[1][2] == 0.0f);
    REQUIRE(identity[2][0] == 0.0f);
    REQUIRE(identity[2][1] == 0.0f);
    REQUIRE(identity[2][2] == 1.0f);

    // [1, 0, 0]
    // [0, 2, 0]
    // [0, 0, 3]
    Matrix3 diagonal(1.0f, 2.0f, 3.0f);
    REQUIRE(diagonal[0][0] == 1.0f);
    REQUIRE(diagonal[0][1] == 0.0f);
    REQUIRE(diagonal[0][2] == 0.0f);
    REQUIRE(diagonal[1][0] == 0.0f);
    REQUIRE(diagonal[1][1] == 2.0f);
    REQUIRE(diagonal[1][2] == 0.0f);
    REQUIRE(diagonal[2][0] == 0.0f);
    REQUIRE(diagonal[2][1] == 0.0f);
    REQUIRE(diagonal[2][2] == 3.0f);

    // [1, 4, 7]
    // [2, 5, 8]
    // [3, 6, 9]
    Matrix3 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
    REQUIRE(a[0] == Vector3(1.0f, 2.0f, 3.0f));
    REQUIRE(a[1] == Vector3(4.0f, 5.0f, 6.0f));
    REQUIRE(a[2] == Vector3(7.0f, 8.0f, 9.0f));

    Matrix3 b(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f), Vector3(7.0f, 8.0f, 9.0f));
    REQUIRE(a == b);
}

TEST_CASE("Matrix3 determinant", "[Matrix3]") {
    // [0, 0, 0]
    // [0, 0, 0]
    // [0, 0, 0]
    Matrix3 zero;
    REQUIRE(zero.determinant() == 0);
    REQUIRE(zero.transposed().determinant() == zero.determinant());

    // [1, 0, 0]
    // [0, 1, 0]
    // [0, 0, 1]
    Matrix3 identity(1.0f);
    REQUIRE(identity.determinant() == 1.0f);
    REQUIRE(identity.transposed().determinant() == identity.determinant());

    // [1, 0, 0]
    // [0, 2, 0]
    // [0, 0, 3]
    Matrix3 diagonal(1.0f, 2.0f, 3.0f);
    REQUIRE(diagonal.determinant() == 6.0f);
    REQUIRE(diagonal.transposed().determinant() == diagonal.determinant());

    // [2, 4, 3]
    // [3, 3, 3]
    // [3, 2, 2]
    Matrix3 a(2.0f, 3.0f, 3.0f, 4.0f, 3.0f, 2.0f, 3.0f, 3.0f, 2.0f);
    REQUIRE(a.determinant() == 3.0f);
    REQUIRE(a.transposed().determinant() == a.determinant());
}

TEST_CASE("Matrix3 transpose", "[Matrix3]") {
    // [0, 0, 0]
    // [0, 0, 0]
    // [0, 0, 0]
    Matrix3 zero;
    Matrix3 zeroTransposed = zero.transposed();
    zero.transpose();
    REQUIRE(zero == Matrix3());
    REQUIRE(zero == zeroTransposed);

    // [1, 0, 0]
    // [0, 2, 0]
    // [0, 0, 3]
    Matrix3 diagonal(1.0f, 2.0f, 3.0f);
    Matrix3 diagonalTransposed = diagonal.transposed();
    diagonal.transpose();
    REQUIRE(diagonal == Matrix3(1.0f, 2.0f, 3.0f));
    REQUIRE(diagonal == diagonalTransposed);

    // [1, 4, 7]
    // [2, 5, 8]
    // [3, 6, 9]
    Matrix3 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
    Matrix3 aTransposed = a.transposed();
    a.transpose();
    REQUIRE(a == Matrix3(1.0f, 4.0f, 7.0f, 2.0f, 5.0f, 8.0f, 3.0f, 6.0f, 9.0f));
    REQUIRE(a == aTransposed);
}

TEST_CASE("Matrix3 inverse", "[Matrix3]") {
    // [1, 0, 0]
    // [0, 1, 0]
    // [0, 0, 1]
    Matrix3 identity(1.0f);
    Matrix3 identityInversed = identity.inversed();
    REQUIRE(identity == identityInversed);
    REQUIRE((identity * identityInversed) == Matrix3(1.0f));
    identity.inverse();
    REQUIRE(identity == identityInversed);

    // [1, 0, 0]
    // [0, 2, 0]
    // [0, 0, 4]
    Matrix3 diagonal(1.0f, 2.0f, 4.0f);
    Matrix3 diagonalInversed = diagonal.inversed();
    REQUIRE((diagonal * diagonalInversed) == Matrix3(1.0f));
    diagonal.inverse();
    REQUIRE(diagonal == diagonalInversed);
    REQUIRE(diagonal == Matrix3(1.0f, 0.5f, 0.25f));

    // [2, 4, 3]
    // [3, 3, 3]
    // [3, 2, 2]
    Matrix3 a(2.0f, 3.0f, 3.0f, 4.0f, 3.0f, 2.0f, 3.0f, 3.0f, 2.0f);
    Matrix3 aInversed = a.inversed();
    REQUIRE(near(a * aInversed, Matrix3(1.0f)));
    REQUIRE(near(aInversed * a, Matrix3(1.0f)));
    a.inverse();
    REQUIRE(a == aInversed);
    REQUIRE(near(a[0], Vector3(0, 1, -1)));
    REQUIRE(near(a[1], Vector3(-2.0f / 3.0f, -5.0f / 3.0f, 8.0f / 3.0f)));
    REQUIRE(near(a[2], Vector3(1, 1, -2)));
}

TEST_CASE("2D point translate", "[Matrix3]") {
    // (1, 0)
    Vector3 a(1.0f, 0.0f, 1.0f);

    Matrix3 trans(1.0f);
    trans.translate(1.0f, 0.0f);
    REQUIRE(near(a * trans, Vector3(2.0f, 0.0f, 1.0f)));

    trans.translate(0.0f, 1.0f);
    REQUIRE(near(a * trans, Vector3(2.0f, 1.0f, 1.0f)));

    Matrix3 trans2 = Matrix3(1.0f).translated(1.0f, 0.0f) * Matrix3(1.0f).translated(0.0f, 1.0f);
    REQUIRE(near(trans, trans2));
    REQUIRE(near(a * trans2, Vector3(2.0f, 1.0f, 1.0f)));
}

TEST_CASE("2D point rotate", "[Matrix3]") {
    // (1, 0)
    Vector3 a(1.0f, 0.0f, 1.0f);

    Matrix3 rotate(1.0f);
    rotate.rotate(Pi<float> / 2.0f);
    REQUIRE(near(a * rotate, Vector3(0.0f, 1.0f, 1.0f)));

    rotate.rotate(Pi<float> / 4.0f);
    REQUIRE(near(a * rotate, Vector3(-1.0f / Sqrt2<float>, 1.0f / Sqrt2<float>, 1.0f)));

    Matrix3 rotate2 =
        Matrix3(1.0f).rotated(Pi<float> / 4.0f) * Matrix3(1.0f).rotated(Pi<float> / 2.0f);
    REQUIRE(near(rotate, rotate2));
    REQUIRE(near(a * rotate2, Vector3(-1.0f / Sqrt2<float>, 1.0f / Sqrt2<float>, 1.0f)));
}

TEST_CASE("2D vector scale", "[Matrix3]") {
    Matrix3 s0(1.0f);
    Matrix3 s1 = s0.scaled(0.5f, 0.5f);
    s0.scale(0.5f, 0.5f);
    REQUIRE(s0 == s1);

    Matrix3 s2(1.0f);
    Matrix3 s3 = s2.scaled(Vector2(0.5f, 0.5f));
    s2.scale(Vector2(0.5f, 0.5f));
    REQUIRE(s2 == s3);
    REQUIRE(s1 == s2);

    Vector3 v0(1.0f, -1.0f, 0.0f);
    REQUIRE((v0 * s0) == Vector3(0.5f, -0.5f, 0.0f));
}

TEST_CASE("2D point shear", "[Matrix3]") {
    SECTION("Shear X") {
        Matrix3 s0(1.0f);
        Matrix3 s1 = s0.shearedX(0.5f);
        s0.shearX(0.5f);
        REQUIRE(s0 == s1);

        Vector3 v0(0.0f, 1.0f, 1.0f);
        REQUIRE((v0 * s0) == Vector3(0.5f, 1.0f, 1.0f));
    }

    SECTION("Shear Y") {
        Matrix3 s0(1.0f);
        Matrix3 s1 = s0.shearedY(0.5f);
        s0.shearY(0.5f);
        REQUIRE(s0 == s1);

        Vector3 v0(1.0f, 0.0f, 1.0f);
        REQUIRE((v0 * s0) == Vector3(1.0f, 0.5f, 1.0f));
    }
}

TEST_CASE("2D transformation", "[Matrix3]") {
    Vector3 p0(1.0f, 0.0f, 1.0f);

    Matrix3 t0(1.0f);
    t0.scale(Sqrt2<float>, 1.0f).rotate(Pi<float> * 0.25f).translate(1.0f, 0.0f);

    Vector3 p1 = p0 * t0;
    REQUIRE(near(p1, Vector3(2.0f, 1.0f, 1.0f)));

    Matrix3 t1 = Matrix3(1.0f).scaled(Sqrt2<float>, 1.0f) *
                 Matrix3(1.0f).rotated(Pi<float> * 0.25f) * Matrix3(1.0f).translated(1.0f, 0.0f);
    REQUIRE(t0 == t1);

    Vector3 p2 = p1 * t0.inversed();
    REQUIRE(near(p0, p2));
}

TEST_CASE("Matrix3 add operator", "[Matrix3]") {
    SECTION("Add scalars") {
        Matrix3 zero;
        Matrix3 a = zero + 1.0f;
        Matrix3 b = 1.0f + zero;
        REQUIRE(a == b);
        REQUIRE(a[0] == Vector3(1.0f));
        REQUIRE(a[1] == Vector3(1.0f));
        REQUIRE(a[2] == Vector3(1.0f));

        // [1, 4, 7]
        // [2, 5, 8]
        // [3, 6, 9]
        Matrix3 c(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);

        Matrix3 d = c + 1.0f;
        c += 1.0f;
        REQUIRE(c == d);
        REQUIRE(c[0] == Vector3(2.0f, 3.0f, 4.0f));
        REQUIRE(c[1] == Vector3(5.0f, 6.0f, 7.0f));
        REQUIRE(c[2] == Vector3(8.0f, 9.0f, 10.0f));
    }

    SECTION("Add matrices") {
        Matrix3 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
        Matrix3 b(9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
        Matrix3 c = a + b;
        a += b;
        REQUIRE(a == c);
        REQUIRE(a == (Matrix3() + 10.0f));
    }
}

TEST_CASE("Matrix3 subtract", "[Matrix3]") {
    SECTION("Subtract scalars") {
        Matrix3 zero;
        Matrix3 a = zero - 1.0f;
        Matrix3 b = 1.0f - zero;
        REQUIRE(a != b);
        REQUIRE(a == -b);
        REQUIRE(a[0] == Vector3(-1.0f));
        REQUIRE(a[1] == Vector3(-1.0f));
        REQUIRE(a[2] == Vector3(-1.0f));
        REQUIRE(b[0] == Vector3(1.0f));
        REQUIRE(b[1] == Vector3(1.0f));
        REQUIRE(b[2] == Vector3(1.0f));

        // [1, 4, 7]
        // [2, 5, 8]
        // [3, 6, 9]
        Matrix3 c(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
        Matrix3 d = c - 1.0f;
        c -= 1.0f;
        REQUIRE(c == d);
        REQUIRE(c == Matrix3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f));
    }

    SECTION("Subtract matrices") {
        Matrix3 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
        Matrix3 b(9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
        Matrix3 c = a - b;
        a -= b;
        REQUIRE(a == c);
        REQUIRE(a == Matrix3(-8.0f, -6.0f, -4.0f, -2.0f, 0.0f, 2.0f, 4.0f, 6.0f, 8.0f));
    }
}

TEST_CASE("Matrix3 multiply", "[Matrix3]") {
    SECTION("Multiply scalars") {
        Matrix3 a(1.0f);
        Matrix3 b = 2.0f * a;
        Matrix3 c = a * 2.0f;
        a *= 2.0f;
        REQUIRE(a == b);
        REQUIRE(a == c);
        REQUIRE(a == Matrix3(2.0f));
    }

    SECTION("Multiply vectors") {
        Matrix3 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
        Vector3 b(1.0f, 2.0f, 3.0f);

        Vector3 c = b * a;
        REQUIRE(c == Vector3(14.0f, 32.0f, 50.0f));

        Vector3 d = a * b;
        REQUIRE(d == Vector3(30.0f, 36.0f, 42.0f));
    }

    SECTION("Multiply matrices") {
        Matrix3 zero;
        Matrix3 identity(1.0f);

        Matrix3 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
        REQUIRE((a * zero) == zero);
        REQUIRE((zero * a) == zero);

        REQUIRE((a * identity) == a);
        REQUIRE((identity * a) == a);

        Matrix3 b(1.0f, -1.0f, 2.0f, 3.0f, 2.0f, 1.0f, -2.0f, 3.0f, 1.0f);
        Matrix3 c = a * b;
        a *= b;
        REQUIRE(a == c);
        REQUIRE(a[0] == Vector3(11.0f, 13.0f, 15.0f));
        REQUIRE(a[1] == Vector3(18.0f, 24.0f, 30.0f));
        REQUIRE(a[2] == Vector3(17.0f, 19.0f, 21.0f));
    }
}

TEST_CASE("Matrix3 divide", "[Matrix3]") {
    SECTION("Divide by scalars") {
        Matrix3 identity(1.0f);
        Matrix3 a = identity / 2.0f;
        REQUIRE(a == Matrix3(0.5f));

        Matrix3 b(2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f);
        Matrix3 c = 1.0f / b;
        REQUIRE(c == Matrix3(0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f));
    }

    SECTION("Divide by matrices") {
        Matrix3 identity(1.0f);
        Matrix3 a(1.0f, -1.0f, 2.0f, 3.0f, 2.0f, 1.0f, -2.0f, 3.0f, 1.0f);
        REQUIRE(near(a / a, identity));
        REQUIRE(near(identity / a, a.inversed()));
    }
}
