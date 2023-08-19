#include <ink/math/vector.hpp>

using namespace ink;

static auto near(float a, float b, float eps = FLT_EPSILON) noexcept -> bool {
    return std::abs(a - b) <= eps;
}

TEST_CASE("Vector2 comparision operators", "[Vector2]") {
    Vector2 a(1.0f);
    Vector2 b(1.0f);
    REQUIRE(a == b);
    REQUIRE(!(a != b));
}

TEST_CASE("Vector2 add operator", "[Vector2]") {
    SECTION("Add scalars") {
        Vector2 a(1.0f);
        Vector2 b = a + 1.0f;
        a += 1.0f;
        REQUIRE(a == b);
        REQUIRE((a.x == 2.0f && a.y == 2.0f));
    }

    SECTION("Add vectors") {
        Vector2 a(1.0f);
        Vector2 b(2.0f, -1.0f);
        Vector2 c = a + b;
        a += b;
        REQUIRE(a == c);
        REQUIRE((a.x == 3.0f && a.y == 0));
    }
}

TEST_CASE("Vector2 subtract operator", "[Vector2]") {
    SECTION("Subtract scalars") {
        Vector2 a(1.0f);
        Vector2 b = a - 1.0f;
        a -= 1.0f;
        REQUIRE(a == b);
        REQUIRE((a.x == 0 && a.y == 0));
    }

    SECTION("Subtract vectors") {
        Vector2 a(1.0f);
        Vector2 b(2.0f, -1.0f);
        Vector2 c = a - b;
        a -= b;
        REQUIRE(a == c);
        REQUIRE((a.x == -1.0f && a.y == 2.0f));
    }
}

TEST_CASE("Vector2 multiply operator", "[Vector2]") {
    SECTION("Multiply scalars") {
        Vector2 a(1.0f);
        Vector2 b = a * 2.0f;
        a *= 2.0f;
        REQUIRE(a == b);
        REQUIRE((a.x == 2.0f && a.y == 2.0f));
    }

    SECTION("Multiply vectors") {
        Vector2 a(1.0f);
        Vector2 b(2.0f, -1.0f);
        Vector2 c = a * b;
        a *= b;
        REQUIRE(a == c);
        REQUIRE((a.x == 2.0f && a.y == -1.0f));
    }
}

TEST_CASE("Vector2 divide operator", "[Vector2]") {
    SECTION("Divide scalars") {
        Vector2 a(1.0f);
        Vector2 b = a / 2.0f;
        a /= 2.0f;
        REQUIRE(a == b);
        REQUIRE((a.x == 0.5f && a.y == 0.5f));
    }

    SECTION("Divide vectors") {
        Vector2 a(1.0f);
        Vector2 b(2.0f, -1.0f);
        Vector2 c = a / b;
        a /= b;
        REQUIRE(a == c);
        REQUIRE(a == Vector2(0.5f, -1.0f));
    }
}

TEST_CASE("Vector2 dot product", "[Vector2]") {
    Vector2 a(1.0f);
    REQUIRE(dot(a, a) == 2.0f);
    Vector2 b(2.0f, 4.0f);
    REQUIRE(dot(a, b) == 6.0f);

    REQUIRE(near(b.length() * b.length(), dot(b, b)));
}

TEST_CASE("Vector2 lerp", "[Vector2]") {
    Vector2 a(0.0f, 1.0f);
    Vector2 b(1.0f, 0.0f);
    REQUIRE(lerp(a, b, 0) == a);
    REQUIRE(lerp(a, b, 1) == b);
    REQUIRE(lerp(a, b, 0.5f) == Vector2(0.5f, 0.5f));
}

TEST_CASE("Vector2 element-wise abs", "[Vector2]") {
    Vector2 positive(1.0f);
    REQUIRE(abs(positive) == positive);

    Vector2 a(1.0f, -1.0f);
    REQUIRE(abs(a) == positive);

    Vector2 b(-1.0f, -1.0f);
    REQUIRE(abs(b) == positive);
}

TEST_CASE("Vector2 element-wise min", "[Vector2]") {
    Vector2 a(1.0f, 3.0f);
    Vector2 b(4.0f, 2.0f);
    REQUIRE(min(a, a) == a);
    REQUIRE(min(a, b) == Vector2(1.0f, 2.0f));
}

TEST_CASE("Vector2 element-wise max", "[Vector2]") {
    Vector2 a(1.0f, 3.0f);
    Vector2 b(4.0f, 2.0f);
    REQUIRE(max(a, a) == a);
    REQUIRE(max(a, b) == Vector2(4.0f, 3.0f));
}

TEST_CASE("Vector2 element-wise clamp", "[Vector2]") {
    Vector2 floor(2.0f, 2.0f);
    Vector2 ceil(3.0f);

    Vector2 a(1.0f, 3.0f);
    REQUIRE(clamp(a, floor, ceil) == Vector2(2.0f, 3.0f));

    Vector2 b(0.0f, 4.0f);
    REQUIRE(clamp(b, floor, ceil) == Vector2(2.0f, 3.0f));

    Vector2 c(2.5f, 2.5f);
    REQUIRE(clamp(c, floor, ceil) == c);
}
