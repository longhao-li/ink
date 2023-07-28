#include <catch_amalgamated.hpp>
#include <ink/math/quaternion.h>

using namespace ink;

static auto near(float a, float b, float eps = FLT_EPSILON) noexcept -> bool {
    return std::abs(a - b) <= eps;
}

static auto near(Quaternion a, Quaternion b, float eps = FLT_EPSILON) noexcept -> bool {
    return near(a.w, b.w, eps) && near(a.x, b.x, eps) && near(a.y, b.y, eps) && near(a.z, b.z, eps);
}

TEST_CASE("Quaternion construct", "[Quaternion]") {
    Quaternion zero;
    REQUIRE(zero.w == 0);
    REQUIRE(zero.x == 0);
    REQUIRE(zero.y == 0);
    REQUIRE(zero.z == 0);

    Quaternion identity(1.0f);
    REQUIRE(identity.w == 1.0f);
    REQUIRE(identity.x == 0);
    REQUIRE(identity.y == 0);
    REQUIRE(identity.z == 0);

    Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(a.w == 1.0f);
    REQUIRE(a.x == 2.0f);
    REQUIRE(a.y == 3.0f);
    REQUIRE(a.z == 4.0f);

    // pitch, yaw, roll
    Quaternion b(0.524f, 1.047f, 0.785f);
    REQUIRE(near(b.w, 0.7233789f));
    REQUIRE(near(b.x, 0.3919574f));
    REQUIRE(near(b.y, 0.360332f));
    REQUIRE(near(b.z, 0.4396058f));
}

TEST_CASE("Quaternion comparision operators", "[Quaternion]") {
    Quaternion a(1.0f);
    Quaternion b(1.0f);
    REQUIRE(a == b);
    REQUIRE(!(a != b));

    Quaternion c(1.0f, 1.0f, 2.0f, 3.0f);
    Quaternion d(0.0f, 1.0f, 2.0f, 3.0f);
    REQUIRE(c == c);
    REQUIRE(c != d);
    c -= a;
    REQUIRE(c == d);
}

TEST_CASE("Quaternion add operator", "[Quaternion]") {
    SECTION("Add real") {
        Quaternion a(1.0f);
        Quaternion b = a + 1.0f;
        a += 1.0f;
        REQUIRE(a == b);
        REQUIRE(a == Quaternion(2.0f));

        Quaternion c(1.0f, 2.0f, 3.0f, 4.0f);
        Quaternion d = c + 1.0f;
        c += 1.0f;
        REQUIRE(c == d);
        REQUIRE(c == Quaternion(2.0f, 2.0f, 3.0f, 4.0f));
    }

    SECTION("Add quaternion") {
        Quaternion a(1.0f);
        Quaternion b(1.0f, -1.0f, 1.0f, -1.0f);
        Quaternion c = a + b;
        a += b;
        REQUIRE(a == c);
        REQUIRE(a == Quaternion(2.0f, -1.0f, 1.0f, -1.0f));
    }
}

TEST_CASE("Quaternion subtract operator", "[Quaternion]") {
    SECTION("Subtract real") {
        Quaternion a(1.0f);
        Quaternion b = a - 1.0f;
        a -= 1.0f;
        REQUIRE(a == b);
        REQUIRE(a == Quaternion(0.0f));

        Quaternion c(1.0f, 2.0f, 3.0f, 4.0f);
        Quaternion d = c - 1.0f;
        c -= 1.0f;
        REQUIRE(c == d);
        REQUIRE(c == Quaternion(0.0f, 2.0f, 3.0f, 4.0f));
    }

    SECTION("Subtract quaternion") {
        Quaternion a(1.0f);
        Quaternion b(1.0f, -1.0f, 1.0f, -1.0f);
        Quaternion c = a - b;
        a -= b;
        REQUIRE(a == c);
        REQUIRE(a == Quaternion(0.0f, 1.0f, -1.0f, 1.0f));
    }
}

TEST_CASE("Quaternion multiply operator", "[Quaternion]") {
    SECTION("Multiply scalars") {
        Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
        Quaternion b = 2.0f * a;
        a *= 2.0f;
        REQUIRE(a == b);
        REQUIRE(a == Quaternion(2.0f, 4.0f, 6.0f, 8.0f));
    }

    SECTION("Multiply quaternion") {
        Quaternion a(1.0f, 2.0f, 3.0f, 4.0f);
        Quaternion b(2.0f, 1.0f, 3.0f, 2.0f);
        Quaternion c = a * b;
        a *= b;
        REQUIRE(a == c);
        REQUIRE(a == Quaternion(-17.0f, -1.0f, 9.0f, 13.0f));
    }
}

TEST_CASE("Quaternion divide operator", "[Quaternion]") {
    SECTION("Divide by scalars") {
        Quaternion identity(1.0f);
        Quaternion a = identity / 2.0f;
        REQUIRE(a == 0.5f);
    }

    SECTION("Divide by quaternions") {
        Quaternion b(0.7233789f, 0.3919574f, 0.360332f, 0.4396058f);
        Quaternion c = 1.0f / b;
        REQUIRE(near(c, Quaternion(0.7233789f, -0.3919574f, -0.360332f, -0.4396058f)));
    }
}
