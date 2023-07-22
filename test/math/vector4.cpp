#include <catch_amalgamated.hpp>
#include <ink/math/vector.h>

using namespace ink;

static auto near(float a, float b, float eps = FLT_EPSILON) noexcept -> bool {
    return std::abs(a - b) <= eps;
}

TEST_CASE("Vector4 comparision operators", "[Vector4]") {
    Vector4 a(1.0f);
    Vector4 b(1.0f);
    REQUIRE(a == b);
    REQUIRE(!(a != b));

    Vector4 c(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 d(0.0f, 1.0f, 2.0f, 3.0f);
    REQUIRE(c == c);
    REQUIRE(c != d);
    c -= a;
    REQUIRE(c == d);
}

TEST_CASE("Vector4 add operator", "[Vector4]") {
    SECTION("Add scalars") {
        Vector4 a(1.0f);
        Vector4 b = a + 1.0f;
        a += 1.0f;
        REQUIRE(a == b);
        REQUIRE(a == Vector4(2.0f));
    }

    SECTION("Add vectors") {
        Vector4 a(1.0f);
        Vector4 b(1.0f, -1.0f, 1.0f, -1.0f);
        Vector4 c = a + b;
        a += b;
        REQUIRE(a == c);
        REQUIRE(a == Vector4(2.0f, 0.0f, 2.0f, 0.0f));
    }
}

TEST_CASE("Vector4 subtract operator", "[Vector4]") {
    SECTION("Subtract scalars") {
        Vector4 a(1.0f);
        Vector4 b = a - 1.0f;
        a -= 1.0f;
        REQUIRE(a == b);
        REQUIRE(a == Vector4());
    }

    SECTION("Subtract vectors") {
        Vector4 a(1.0f);
        Vector4 b(1.0f, -1.0f, 1.0f, -1.0f);
        Vector4 c = a - b;
        a -= b;
        REQUIRE(a == c);
        REQUIRE(a == Vector4(0, 2.0f, 0, 2.0f));
    }
}

TEST_CASE("Vector4 multiply operator", "[Vector4]") {
    SECTION("Multiply scalars") {
        Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
        Vector4 b = 2.0f * a;
        a *= 2.0f;
        REQUIRE(a == b);
        REQUIRE(a == Vector4(2.0f, 4.0f, 6.0f, 8.0f));
    }

    SECTION("Multiply vectors") {
        Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
        Vector4 b(4.0f, 3.0f, 2.0f, 1.0f);
        Vector4 c = a * b;
        a *= b;
        REQUIRE(a == c);
        REQUIRE(a == Vector4(4.0f, 6.0f, 6.0f, 4.0f));
    }
}

TEST_CASE("Vector4 divide operator", "[Vector4]") {
    SECTION("Divide scalars") {
        Vector4 a(1.0f, 2.0f, 1.0f, 2.0f);
        Vector4 b = a / 2.0f;
        Vector4 c = 2.0f / a;
        a /= 2.0f;
        REQUIRE(a == b);
        REQUIRE(a == Vector4(0.5f, 1.0f, 0.5f, 1.0f));
        REQUIRE((a * c == Vector4(1.0f)));
    }
}

TEST_CASE("Vector4 dot product", "[Vector4]") {
    Vector4 a(1.0f);
    REQUIRE(dot(a, a) == 4.0f);
    REQUIRE(near(a.length(), 2.0f));

    Vector4 b(1.0f, -1.0f, 1.0f, -1.0f);
    REQUIRE(dot(a, a) == dot(b, b));

    Vector4 c(2.0f, 3.0f, 3.0f, 3.0f);
    REQUIRE(dot(b, c) == dot(c, b));
    REQUIRE(dot(b, c) == -1.0f);
}

TEST_CASE("Vector4 cross product", "[Vector4]") {
    Vector4 x(1.0f, 0.0f, 0.0f, 0.0f);
    Vector4 y(0.0f, 1.0f, 0.0f, 0.0f);
    Vector4 z(0.0f, 0.0f, 1.0f, 0.0f);
    REQUIRE(cross(x, y) == z);
    REQUIRE(cross(y, z) == x);
    REQUIRE(cross(z, x) == y);

    Vector4 a(4.0f, 6.0f, -2.0f, 0.0f);
    Vector4 b(-3.0f, 1.0f, 5.0f, 0.0f);
    Vector4 c = cross(a, b);

    REQUIRE(c == Vector4(32.0f, -14.0f, 22.0f, 0.0f));
    REQUIRE(near(dot(a, c), 0));
    REQUIRE(near(dot(b, c), 0));
}

TEST_CASE("Vector4 lerp", "[Vector4]") {
    Vector4 a(1.0f, -1.0f, 1.0f, -1.0f);
    Vector4 b(-1.0f, 1.0f, -1.0f, 1.0f);
    REQUIRE(lerp(a, b, 0.0f) == a);
    REQUIRE(lerp(a, b, 1.0f) == b);
    REQUIRE(lerp(a, b, 0.5f) == Vector4());
}

TEST_CASE("Vector4 element-wise abs", "[Vector4]") {
    Vector4 positive(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(abs(positive) == positive);

    Vector4 a(1.0f, -1.0f, 1.0f, -1.0f);
    REQUIRE(abs(a) == Vector4(1.0f));

    Vector4 b;
    REQUIRE(abs(b) == Vector4());
}

TEST_CASE("Vector4 element-wise min", "[Vector4]") {
    Vector4 a(1.0f, -1.0f, 1.0f, -1.0f);
    Vector4 b(2.0f, -2.0f, 2.0f, -2.0f);
    REQUIRE(min(a, a) == a);
    REQUIRE(min(b, b) == b);
    REQUIRE(min(a, b) == Vector4(1.0f, -2.0f, 1.0f, -2.0f));
}

TEST_CASE("Vector4 element-wise max", "[Vector4]") {
    Vector4 a(1.0f, -1.0f, 1.0f, -1.0f);
    Vector4 b(2.0f, -2.0f, 2.0f, -2.0f);
    REQUIRE(max(a, a) == a);
    REQUIRE(max(b, b) == b);
    REQUIRE(max(a, b) == Vector4(2.0f, -1.0f, 2.0f, -1.0f));
}

TEST_CASE("Vector4 element-wise clamp", "[Vector4]") {
    Vector4 floor(-1.0f);
    Vector4 ceil(1.0f);

    Vector4 a(-2.0f, 1.0f, -1.0f, 2.0f);
    REQUIRE(clamp(a, floor, ceil) == Vector4(-1.0f, 1.0f, -1.0f, 1.0f));
    REQUIRE(clamp(floor, floor, ceil) == floor);
    REQUIRE(clamp(ceil, floor, ceil) == ceil);
    REQUIRE(clamp(Vector4(), floor, ceil) == Vector4());
}
