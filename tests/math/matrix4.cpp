#include <ink/math/matrix.hpp>
#include <ink/math/numbers.hpp>

using namespace ink;

static auto near(float a, float b, float eps = FLT_EPSILON) noexcept -> bool {
    return std::abs(a - b) <= eps;
}

static auto near(Vector4 a, Vector4 b, float eps = FLT_EPSILON) noexcept -> bool {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps) && near(a.z, b.z, eps) && near(a.w, b.w, eps);
}

static auto near(const Matrix4 &a, const Matrix4 &b, float eps = FLT_EPSILON) noexcept -> bool {
    return near(a[0], b[0], eps) && near(a[1], b[1], eps) && near(a[2], b[2], eps) &&
           near(a[3], b[3], eps);
}

TEST_CASE("Matrix4 construct", "[Matrix4]") {
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    Matrix4 zero;
    REQUIRE(zero[0] == Vector4());
    REQUIRE(zero[1] == Vector4());
    REQUIRE(zero[2] == Vector4());
    REQUIRE(zero[3] == Vector4());

    // [1, 0, 0, 0]
    // [0, 1, 0, 0]
    // [0, 0, 1, 0]
    // [0, 0, 0, 1]
    Matrix4 identity(1.0f);
    REQUIRE(identity[0][0] == 1.0f);
    REQUIRE(identity[0][1] == 0.0f);
    REQUIRE(identity[0][2] == 0.0f);
    REQUIRE(identity[0][3] == 0.0f);
    REQUIRE(identity[1][0] == 0.0f);
    REQUIRE(identity[1][1] == 1.0f);
    REQUIRE(identity[1][2] == 0.0f);
    REQUIRE(identity[1][3] == 0.0f);
    REQUIRE(identity[2][0] == 0.0f);
    REQUIRE(identity[2][1] == 0.0f);
    REQUIRE(identity[2][2] == 1.0f);
    REQUIRE(identity[2][3] == 0.0f);
    REQUIRE(identity[3][0] == 0.0f);
    REQUIRE(identity[3][1] == 0.0f);
    REQUIRE(identity[3][2] == 0.0f);
    REQUIRE(identity[3][3] == 1.0f);

    // [1, 0, 0, 0]
    // [0, 2, 0, 0]
    // [0, 0, 3, 0]
    // [0, 0, 0, 4]
    Matrix4 diagonal(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(diagonal[0][0] == 1.0f);
    REQUIRE(diagonal[0][1] == 0.0f);
    REQUIRE(diagonal[0][2] == 0.0f);
    REQUIRE(diagonal[0][3] == 0.0f);
    REQUIRE(diagonal[1][0] == 0.0f);
    REQUIRE(diagonal[1][1] == 2.0f);
    REQUIRE(diagonal[1][2] == 0.0f);
    REQUIRE(diagonal[1][3] == 0.0f);
    REQUIRE(diagonal[2][0] == 0.0f);
    REQUIRE(diagonal[2][1] == 0.0f);
    REQUIRE(diagonal[2][2] == 3.0f);
    REQUIRE(diagonal[2][3] == 0.0f);
    REQUIRE(diagonal[3][0] == 0.0f);
    REQUIRE(diagonal[3][1] == 0.0f);
    REQUIRE(diagonal[3][2] == 0.0f);
    REQUIRE(diagonal[3][3] == 4.0f);

    // [1, 5, 9,  13]
    // [2, 6, 10, 14]
    // [3, 7, 11, 15]
    // [4, 8, 12, 16]
    Matrix4 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
              14.0f, 15.0f, 16.0f);
    REQUIRE(a[0] == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    REQUIRE(a[1] == Vector4(5.0f, 6.0f, 7.0f, 8.0f));
    REQUIRE(a[2] == Vector4(9.0f, 10.0f, 11.0f, 12.0f));
    REQUIRE(a[3] == Vector4(13.0f, 14.0f, 15.0f, 16.0f));

    Matrix4 b(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(5.0f, 6.0f, 7.0f, 8.0f),
              Vector4(9.0f, 10.0f, 11.0f, 12.0f), Vector4(13.0f, 14.0f, 15.0f, 16.0f));
    REQUIRE(a == b);
}

TEST_CASE("Matrix4 determinant", "[Matrix4]") {
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    Matrix4 zero;
    REQUIRE(zero.determinant() == 0.0f);
    REQUIRE(zero.transposed().determinant() == zero.determinant());

    // [1, 0, 0, 0]
    // [0, 1, 0, 0]
    // [0, 0, 1, 0]
    // [0, 0, 0, 1]
    Matrix4 identity(1.0f);
    REQUIRE(identity.determinant() == 1.0f);
    REQUIRE(identity.transposed().determinant() == identity.determinant());

    // [1, 0, 0, 0]
    // [0, 2, 0, 0]
    // [0, 0, 3, 0]
    // [0, 0, 0, 4]
    Matrix4 diagonal(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(diagonal.determinant() == 24.0f);
    REQUIRE(diagonal.transposed().determinant() == diagonal.determinant());

    // [2, 3,  1, 2]
    // [3, 7,  0, 3]
    // [1, 0,  1, 3]
    // [2, -1, 0, 3]
    Matrix4 a{
        Vector4(2.0f, 3.0f, 1.0f, 2.0f),
        Vector4(3.0f, 7.0f, 0.0f, -1.0f),
        Vector4(1.0f, 0.0f, 1.0f, 0.0f),
        Vector4(2.0f, 3.0f, 3.0f, 3.0f),
    };
    REQUIRE(a.determinant() == 32.0f);
    REQUIRE(a.transposed().determinant() == a.determinant());
}

TEST_CASE("Matrix4 transpose", "[Matrix4]") {
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    // [0, 0, 0, 0]
    Matrix4 zero;
    Matrix4 zeroTransposed = zero.transposed();
    zero.transpose();
    REQUIRE(zero == Matrix4());
    REQUIRE(zero == zeroTransposed);

    // [1, 0, 0, 0]
    // [0, 2, 0, 0]
    // [0, 0, 3, 0]
    // [0, 0, 0, 4]
    Matrix4 diagonal(1.0f, 2.0f, 3.0f, 4.0f);
    Matrix4 diagonalTransposed = diagonal.transposed();
    diagonal.transpose();
    REQUIRE(diagonal == Matrix4(1.0f, 2.0f, 3.0f, 4.0f));
    REQUIRE(diagonal == diagonalTransposed);

    // [1, 5, 9,  13]
    // [2, 6, 10, 14]
    // [3, 7, 11, 15]
    // [4, 8, 12, 16]
    Matrix4 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
              14.0f, 15.0f, 16.0f);
    Matrix4 aTransposed = a.transposed();
    a.transpose();
    REQUIRE(a == Matrix4(1.0f, 5.0f, 9.0f, 13.0f, 2.0f, 6.0f, 10.0f, 14.0f, 3.0f, 7.0f, 11.0f,
                         15.0f, 4.0f, 8.0f, 12.0f, 16.0f));
    REQUIRE(a == aTransposed);
}

TEST_CASE("Matrix4 inverse", "[Matrix4]") {
    // [1, 0, 0, 0]
    // [0, 1, 0, 0]
    // [0, 0, 1, 0]
    // [0, 0, 0, 1]
    Matrix4 identity(1.0f);
    Matrix4 identityInversed = identity.inversed();
    REQUIRE((identity * identityInversed) == Matrix4(1.0f));
    identity.inverse();
    REQUIRE(identity == identityInversed);
    REQUIRE(identity == Matrix4(1.0f));

    // [1, 0, 0, 0]
    // [0, 2, 0, 0]
    // [0, 0, 4, 0]
    // [0, 0, 0, 8]
    Matrix4 diagonal(1.0f, 2.0f, 4.0f, 8.0f);
    Matrix4 diagonalInversed = diagonal.inversed();
    REQUIRE((diagonal * diagonalInversed) == Matrix4(1.0f));
    diagonal.inverse();
    REQUIRE(diagonal == diagonalInversed);
    REQUIRE(diagonal == Matrix4(1.0f, 0.5f, 0.25f, 0.125f));

    // [2, 3,  1, 2]
    // [3, 7,  0, 3]
    // [1, 0,  1, 3]
    // [2, -1, 0, 3]
    Matrix4 a{
        Vector4(2.0f, 3.0f, 1.0f, 2.0f),
        Vector4(3.0f, 7.0f, 0.0f, -1.0f),
        Vector4(1.0f, 0.0f, 1.0f, 0.0f),
        Vector4(2.0f, 3.0f, 3.0f, 3.0f),
    };
    Matrix4 aInversed = a.inversed();
    REQUIRE((a * aInversed) == Matrix4(1.0f));
    a.inverse();
    REQUIRE(a == aInversed);
    REQUIRE(a[0] == Vector4(3.0f / 4.0f, -3.0f / 32.0f, 27.0f / 32.0f, -17.0f / 32.0f));
    REQUIRE(a[1] == Vector4(-1.0f / 4.0f, 5.0f / 32.0f, -13.0f / 32.0f, 7.0f / 32.0f));
    REQUIRE(a[2] == Vector4(-3.0f / 4.0f, 3.0f / 32.0f, 5.0f / 32.0f, 17.0f / 32.0f));
    REQUIRE(a[3] == Vector4(1.0f / 2.0f, -3.0f / 16.0f, -5.0f / 16.0f, -1.0f / 16.0f));
}

TEST_CASE("3D point translate", "[Matrix4]") {
    // (1, 0, 0)
    Vector4 a(1.0f, 0.0f, 0.0f, 1.0f);

    Matrix4 trans(1.0f);
    trans.translate(1.0f, 0.0f, 0.0f);
    REQUIRE(near(a * trans, Vector4(2.0f, 0.0f, 0.0f, 1.0f)));
    trans.translate(0.0f, 1.0f, 0.0f);
    REQUIRE(near(a * trans, Vector4(2.0f, 1.0f, 0.0f, 1.0f)));
    trans.translate(Vector3(0.0f, 1.0f, 2.0f));
    REQUIRE(near(a * trans, Vector4(2.0f, 2.0f, 2.0f, 1.0f)));

    Matrix4 trans2 = Matrix4(1.0f).translated(1.0f, 0.0f, 0.0f) *
                     Matrix4(1.0f).translated(0.0f, 1.0f, 0.0f) *
                     Matrix4(1.0f).translated(Vector3(0.0f, 1.0f, 2.0f));
    REQUIRE(trans == trans2);
}

TEST_CASE("3D point rotate", "[Matrix4]") {
    // (1, 0, 0)
    Vector4 a(1.0f, 0.0f, 0.0f, 1.0f);

    Matrix4 rotate(1.0f);
    rotate.rotate(Vector3(0.0f, 1.0f, 0.0f), Pi<float> / 2.0f);
    REQUIRE(near(a * rotate, Vector4(0.0f, 0.0f, -1.0f, 1.0f)));
    rotate.rotate(Vector3(1.0f, 0.0f, 0.0f), Pi<float> / 2.0f);
    REQUIRE(near(a * rotate, Vector4(0.0f, 1.0f, 0.0f, 1.0f)));
    rotate.rotate(Vector3(1.0f, 1.0f, 1.0f), Pi<float> * 2.0f / 3.0f);
    REQUIRE(near(a * rotate, Vector4(0.0f, 0.0f, 1.0f, 1.0f), 1.0e-6f));

    Matrix4 rotate2 = Matrix4(1.0f).rotated(Vector3(0.0f, 1.0f, 0.0f), Pi<float> / 2.0f) *
                      Matrix4(1.0f).rotated(Vector3(1.0f, 0.0f, 0.0f), Pi<float> / 2.0f) *
                      Matrix4(1.0f).rotated(Vector3(1.0f, 1.0f, 1.0f), Pi<float> * 2.0f / 3.0f);
    REQUIRE(rotate == rotate2);
}

TEST_CASE("3D vector scale", "[Matrix4]") {
    Matrix4 s0(1.0f);
    Matrix4 s1 = s0.scaled(0.2f, 0.5f, 0.8f);
    s0.scale(0.2f, 0.5f, 0.8f);
    REQUIRE(s0 == s1);

    Matrix4 s2(1.0f);
    Matrix4 s3 = s2.scaled(Vector3(0.2f, 0.5f, 0.8f));
    s2.scale(Vector3(0.2f, 0.5f, 0.8f));
    REQUIRE(s2 == s3);
    REQUIRE(s1 == s2);

    Vector4 v0(1.0f, 2.0f, 3.0f, 0.0f);
    REQUIRE((v0 * s0) == Vector4(0.2f, 1.0f, 2.4f, 0.0f));
}

TEST_CASE("3D transformation", "[Matrix4]") {
    Vector4 p0(1.0f, 0.0f, 0.0f, 1.0f);

    Matrix4 t0(1.0f);
    t0.scale(Sqrt2<float>, 1.0f, 1.0f)
        .rotate(Vector3(0.0f, 0.0f, 1.0f), Pi<float> * 0.25f)
        .translate(0.0f, 0.0f, 1.0f);
    Vector4 p1 = p0 * t0;
    REQUIRE(near(p1, Vector4(1.0f, 1.0f, 1.0f, 1.0f)));

    Matrix4 t1 = Matrix4(1.0f).scaled(Sqrt2<float>, 1.0f, 1.0f) *
                 Matrix4(1.0f).rotated(Vector3(0.0f, 0.0f, 1.0f), Pi<float> * 0.25f) *
                 Matrix4(1.0f).translated(0.0f, 0.0f, 1.0f);
    REQUIRE(t0 == t1);

    Vector4 p2 = p1 * t0.inversed();
    REQUIRE(near(p0, p2));
}

TEST_CASE("Matrix4 add operator", "[Matrix4]") {
    SECTION("Add scalars") {
        Matrix4 zero;
        Matrix4 a = zero + 1.0f;
        Matrix4 b = 1.0f + zero;
        REQUIRE(a == b);
        REQUIRE(a[0] == Vector4(1.0f));
        REQUIRE(a[1] == Vector4(1.0f));
        REQUIRE(a[2] == Vector4(1.0f));
        REQUIRE(a[3] == Vector4(1.0f));

        // [1, 5, 9,  13]
        // [2, 6, 10, 14]
        // [3, 7, 11, 15]
        // [4, 8, 12, 16]
        Matrix4 c(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
                  14.0f, 15.0f, 16.0f);
        Matrix4 d = c + 1.0f;
        c += 1.0f;
        REQUIRE(c == d);
        REQUIRE(c[0] == Vector4(2.0f, 3.0f, 4.0f, 5.0f));
        REQUIRE(c[1] == Vector4(6.0f, 7.0f, 8.0f, 9.0f));
        REQUIRE(c[2] == Vector4(10.0f, 11.0f, 12.0f, 13.0f));
        REQUIRE(c[3] == Vector4(14.0f, 15.0f, 16.0f, 17.0f));
    }

    SECTION("Add matrices") {
        Matrix4 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
                  14.0f, 15.0f, 16.0f);
        Matrix4 b(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f,
                  4.0f, 3.0f, 2.0f, 1.0f);
        Matrix4 c = a + b;
        a += b;
        REQUIRE(a == c);
        REQUIRE(a == (Matrix4() + 17.0f));
    }
}

TEST_CASE("Matrix4 subtract", "[Matrix4]") {
    SECTION("Subtract scalars") {
        Matrix4 zero;
        Matrix4 a = zero - 1.0f;
        Matrix4 b = 1.0f - zero;
        REQUIRE(a != b);
        REQUIRE(a == -b);
        REQUIRE(a[0] == Vector4(-1.0f));
        REQUIRE(a[1] == Vector4(-1.0f));
        REQUIRE(a[2] == Vector4(-1.0f));
        REQUIRE(a[3] == Vector4(-1.0f));
        REQUIRE(b[0] == Vector4(1.0f));
        REQUIRE(b[1] == Vector4(1.0f));
        REQUIRE(b[2] == Vector4(1.0f));
        REQUIRE(b[3] == Vector4(1.0f));

        // [1, 5, 9,  13]
        // [2, 6, 10, 14]
        // [3, 7, 11, 15]
        // [4, 8, 12, 16]
        Matrix4 c(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
                  14.0f, 15.0f, 16.0f);
        Matrix4 d = c - 1.0f;
        c -= 1.0f;
        REQUIRE(c == d);
        REQUIRE(c[0] == Vector4(0.0f, 1.0f, 2.0f, 3.0f));
        REQUIRE(c[1] == Vector4(4.0f, 5.0f, 6.0f, 7.0f));
        REQUIRE(c[2] == Vector4(8.0f, 9.0f, 10.0f, 11.0f));
        REQUIRE(c[3] == Vector4(12.0f, 13.0f, 14.0f, 15.0f));
    }

    SECTION("Subtract matrices") {
        Matrix4 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
                  14.0f, 15.0f, 16.0f);
        Matrix4 b(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f,
                  4.0f, 3.0f, 2.0f, 1.0f);
        Matrix4 c = a - b;
        a -= b;
        REQUIRE(a == c);
        REQUIRE(a[0] == Vector4(-15.0f, -13.0f, -11.0f, -9.0f));
        REQUIRE(a[1] == Vector4(-7.0f, -5.0f, -3.0f, -1.0f));
        REQUIRE(a[2] == Vector4(1.0f, 3.0f, 5.0f, 7.0f));
        REQUIRE(a[3] == Vector4(9.0f, 11.0f, 13.0f, 15.0f));
    }
}

TEST_CASE("Matrix4 multiply", "[Matrix4]") {
    SECTION("Multiply scalars") {
        Matrix4 a(1.0f);
        Matrix4 b = 2.0f * a;
        Matrix4 c = a * 2.0f;
        a *= 2.0f;
        REQUIRE(a == b);
        REQUIRE(a == c);
        REQUIRE(a == Matrix4(2.0f));
    }

    SECTION("Multiply vectors") {
        // [1, 5, 9,  13]
        // [2, 6, 10, 14]
        // [3, 7, 11, 15]
        // [4, 8, 12, 16]
        Matrix4 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
                  14.0f, 15.0f, 16.0f);
        Vector4 b(2.0f, 3.0f, 1.0f, 2.0f);

        Vector4 c = b * a;
        REQUIRE(c == Vector4(19.0f, 51.0f, 83.0f, 115.0f));
        Vector4 d = a * b;
        REQUIRE(d == Vector4(52.0f, 60.0f, 68.0f, 76.0f));
    }

    SECTION("Multiply matrices") {
        // [1, 5, 9,  13]
        // [2, 6, 10, 14]
        // [3, 7, 11, 15]
        // [4, 8, 12, 16]
        Matrix4 a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
                  14.0f, 15.0f, 16.0f);

        // [2, -1,  1,  2]
        // [3,  2,  0, -3]
        // [1, -3,  3,  1]
        // [2,  2, -1, -2]
        Matrix4 b(2.0f, 3.0f, 1.0f, 2.0f, -1.0f, 2.0f, -3.0f, 2.0f, 1.0f, 0.0f, 3.0f, -1.0f, 2.0f,
                  -3.0f, 1.0f, -2.0f);

        Matrix4 c = a * b;
        REQUIRE(c[0] == Vector4(52.0f, 60.0f, 68.0f, 76.0f));
        REQUIRE(c[1] == Vector4(8.0f, 8.0f, 8.0f, 8.0f));
        REQUIRE(c[2] == Vector4(15.0f, 18.0f, 21.0f, 24.0f));
        REQUIRE(c[3] == Vector4(-30.0f, -32.0f, -34.0f, -36.0f));
    }
}

TEST_CASE("Matrix4 divide", "[Matrix4]") {
    SECTION("Divide by scalars") {
        Matrix4 identity(1.0f);
        Matrix4 a = identity / 2.0f;
        REQUIRE(a == Matrix4(0.5f));

        Matrix4 b(2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f,
                  2.0f, 2.0f, 2.0f);
        Matrix4 c = 1.0f / b;
        REQUIRE(c == Matrix4(0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
                             0.5f, 0.5f, 0.5f, 0.5f));
    }

    SECTION("Divide by matrices") {
        Matrix4 identity(1.0f);

        // [2, 3,  1, 2]
        // [3, 7,  0, 3]
        // [1, 0,  1, 3]
        // [2, -1, 0, 3]
        Matrix4 a{
            Vector4(2.0f, 3.0f, 1.0f, 2.0f),
            Vector4(3.0f, 7.0f, 0.0f, -1.0f),
            Vector4(1.0f, 0.0f, 1.0f, 0.0f),
            Vector4(2.0f, 3.0f, 3.0f, 3.0f),
        };

        REQUIRE(near(a / a, identity));
        REQUIRE(near(identity / a, a.inversed()));
    }
}
