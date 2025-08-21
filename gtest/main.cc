#include <gtest/gtest.h>
//assert失败，退出当前测试
//expect失败，继续往下执行
int add(int num1,int num2){
    return num1+num2;
}

TEST(calculate,AddIsValid)
{
    ASSERT_EQ(add(1,3),4);
    ASSERT_NE(add(2,0),5);
    ASSERT_EQ(add(4,3),7);
    ASSERT_EQ(add(1,3),4);
}
TEST(string,isEqual)
{
    std::string s = "hello";
    EXPECT_EQ("hello",s);

}

int main(int argc,char *argv[])
{
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}