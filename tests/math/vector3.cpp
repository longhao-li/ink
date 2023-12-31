#include <ink/math/vector.hpp>

using namespace ink;

static auto near(float a, float b, float eps = FLT_EPSILON) noexcept -> bool {
    return std::abs(a - b) <= eps;
}

TEST_CASE("Vector3 comparision operators", "[Vector3]") {
    Vector3 a(1.0f);
    Vector3 b(1.0f, Vector2(1.0f, 1.0f));
    REQUIRE(a == b);
    REQUIRE(!(a != b));
}

TEST_CASE("Vector3 add operator", "[Vector3]") {
    SECTION("Add scalars") {
        Vector3 a(1.0f);
        Vector3 b = a + 1.0f;
        a += 1.0f;
        REQUIRE(a == b);
        REQUIRE(a == Vector3(2.0f));
    }

    SECTION("Add vectors") {
        Vector3 a(1.0f);
        Vector3 b(1.0f, 2.0f, 3.0f);
        Vector3 c = a + b;
        a += b;
        REQUIRE(a == c);
        REQUIRE(a == Vector3(2.0f, 3.0f, 4.0f));
    }
}

TEST_CASE("Vector3 subtract operator", "[Vector3]") {
    SECTION("Subtract scalars") {
        Vector3 a(1.0f, 2.0f, 3.0f);
        Vector3 b = a - 1.0f;
        a -= 1.0f;
        REQUIRE(a == b);
        REQUIRE(a == Vector3(0.0f, 1.0f, 2.0f));
    }

    SECTION("Subtract vectors") {
        Vector3 a(1.0f);
        Vector3 b(1.0f, 2.0f, 3.0f);
        Vector3 c = a - b;
        a -= b;
        REQUIRE(a == c);
        REQUIRE(a == Vector3(0.0f, -1.0f, -2.0f));
    }
}

TEST_CASE("Vector3 multiply operator", "[Vector3]") {
    SECTION("Multiply scalars") {
        Vector3 a(1.0f, 3.0f, 5.0f);
        Vector3 b = a * 0.5f;
        a *= 0.5f;
        REQUIRE(a == b);
        REQUIRE(a == Vector3(0.5f, 1.5f, 2.5f));
    }

    SECTION("Multiply vectors") {
        Vector3 a(2.0f);
        Vector3 b(2.0f, 3.0f, 4.0f);
        Vector3 c = a * b;
        a *= b;
        REQUIRE(a == c);
        REQUIRE(a == b * 2.0f);
        REQUIRE(a == Vector3(4.0f, 6.0f, 8.0f));
    }
}

TEST_CASE("Vector3 divide operator", "[Vector3]") {
    SECTION("Divide scalars") {
        Vector3 a(1.0f);
        Vector3 b = a / 2.0f;
        a /= 2.0f;
        REQUIRE(a == b);
        REQUIRE(a == Vector3(0.5f));
    }

    SECTION("Divide vectors") {
        Vector3 a(1.0f);
        Vector3 b(2.0f, 4.0f, 1.0f);
        Vector3 c = a / b;
        a /= b;
        REQUIRE(a == c);
        REQUIRE(a == Vector3(0.5f, 0.25f, 1.0f));
    }
}

TEST_CASE("Vector3 dot product", "[Vector3]") {
    Vector3 a(1.0f);
    REQUIRE(dot(a, a) == 3.0f);

    Vector3 b(1.0f, 2.0f, 3.0f);
    REQUIRE(dot(a, b) == 6.0f);

    REQUIRE(near(a.length() * a.length(), dot(a, a)));
}

TEST_CASE("Vector3 cross product", "[Vector3]") {
    Vector3 x(1.0f, 0.0f, 0.0f);
    Vector3 y(0.0f, 1.0f, 0.0f);
    Vector3 z(0.0f, 0.0f, 1.0f);
    REQUIRE(cross(x, y) == z);
    REQUIRE(cross(y, z) == x);
    REQUIRE(cross(z, x) == y);

    Vector3 a(4.0f, 6.0f, -2.0f);
    Vector3 b(-3.0f, 1.0f, 5.0f);
    Vector3 c = cross(a, b);

    REQUIRE(c == Vector3(32.0f, -14.0f, 22.0f));
    REQUIRE(near(dot(a, c), 0));
    REQUIRE(near(dot(b, c), 0));
}

TEST_CASE("Vector3 lerp", "[Vector3]") {
    Vector3 a(1.0f, 0.0f, -1.0f);
    Vector3 b(0.0f, -1.0f, 1.0f);
    REQUIRE(lerp(a, b, 0) == a);
    REQUIRE(lerp(a, b, 1) == b);
    REQUIRE(lerp(a, b, 0.5f) == Vector3(0.5f, -0.5f, 0));
}

TEST_CASE("Vector3 element-wise abs", "[Vector3]") {
    Vector3 positive(1.0f, 1.0f, 1.0f);
    REQUIRE(abs(positive) == positive);
    Vector3 a(1.0f, -1.0f, 1.0f);
    REQUIRE(abs(a) == positive);
    Vector3 b(-1.0f, -1.0f, -1.0f);
    REQUIRE(abs(b) == positive);
}

TEST_CASE("Vector3 element-wise min", "[Vector3]") {
    Vector3 a(1.0f, 4.0f, 2.0f);
    Vector3 b(2.0f, 0.0f, 1.0f);
    REQUIRE(min(a, a) == a);
    REQUIRE(min(a, b) == Vector3(1.0f, 0.0f, 1.0f));
}

TEST_CASE("Vector3 element-wise max", "[Vector3]") {
    Vector3 a(1.0f, 4.0f, 2.0f);
    Vector3 b(2.0f, 0.0f, 1.0f);
    REQUIRE(max(a, a) == a);
    REQUIRE(max(a, b) == Vector3(2.0f, 4.0f, 2.0f));
}

TEST_CASE("Vector3 element-wise clamp", "[Vector3]") {
    Vector3 floor(1.0f, 1.0f, 1.0f);
    Vector3 ceil(3.0f, 3.0f, 3.0f);
    Vector3 a(0.0f, 2.0f, 4.0f);
    Vector3 b(0.0f);
    Vector3 c(4.0f);
    REQUIRE(clamp(a, floor, ceil) == Vector3(1.0f, 2.0f, 3.0f));
    REQUIRE(clamp(b, floor, ceil) == floor);
    REQUIRE(clamp(c, floor, ceil) == ceil);
    REQUIRE(clamp(floor, floor, ceil) == floor);
    REQUIRE(clamp(ceil, floor, ceil) == ceil);
}
