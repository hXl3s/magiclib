#include <gtest/gtest.h>

#define EIGEN_USE_MKL_ALL
#include <Eigen/Dense>

TEST(GTestFrameworkTest, GTestTrue) {
    ASSERT_EQ(true, true);
    ASSERT_EQ(true, static_cast<bool>(1));
}

TEST(GTestFrameworkTest, GTestFalse) {
    ASSERT_EQ(false, static_cast<bool>(0));
}

TEST(GTestFrameworkTest, GTest_ForceEigen_MKL) {
    Eigen::MatrixXd mat(2, 2);
    mat(0, 0) = mat(1, 1) = 2;
    mat(1, 0) = mat(0, 1) = 1;

    mat = mat * mat;
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
