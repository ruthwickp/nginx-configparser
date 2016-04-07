#include "gtest/gtest.h"
#include "config_parser.h"

#include <iostream>
using namespace std;


#include <sstream>
#include <string>

class NginxStringConfigTest : public ::testing::Test {
protected:
    bool ParseString(const std::string& config_string) {
        std::stringstream config_stream(config_string);
        return parser_.Parse(&config_stream, &out_config_);
    }
    NginxConfigParser parser_;
    NginxConfig out_config_;
};

TEST_F(NginxStringConfigTest, SimpleConfig) {
    EXPECT_TRUE(ParseString("foo bar;"));
    EXPECT_EQ(1, out_config_.statements_.size());
    EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", out_config_.statements_.at(0)->tokens_.at(1));
}

TEST_F(NginxStringConfigTest, EmptyConfig) {
    EXPECT_FALSE(ParseString(""));
}

TEST_F(NginxStringConfigTest, ExtraSpacesConfig) {
    EXPECT_TRUE(ParseString("   foo    bar   ;   "));
    EXPECT_EQ(1, out_config_.statements_.size());
    EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", out_config_.statements_.at(0)->tokens_.at(1));
}

TEST_F(NginxStringConfigTest, CommentConfig) {
    EXPECT_FALSE(ParseString("// Comment"));
}

TEST_F(NginxStringConfigTest, MulitStatementConfig) {
    EXPECT_TRUE(ParseString("foo bar; listen  80;"));
    EXPECT_EQ(2, out_config_.statements_.size());
    EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("bar", out_config_.statements_.at(0)->tokens_.at(1));
    EXPECT_EQ("listen", out_config_.statements_.at(1)->tokens_.at(0));
    EXPECT_EQ("80", out_config_.statements_.at(1)->tokens_.at(1));
}


TEST_F(NginxStringConfigTest, SpecialCharactersConfig) {
    EXPECT_TRUE(ParseString("root /home/ubuntu/sites/foo/ foo.com;"));
    // We can also add expectations about the contents of the statements:
    EXPECT_EQ(1, out_config_.statements_.size());
    EXPECT_EQ("root", out_config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("/home/ubuntu/sites/foo/", out_config_.statements_.at(0)->tokens_.at(1));
    EXPECT_EQ("foo.com", out_config_.statements_.at(0)->tokens_.at(2));
}

TEST_F(NginxStringConfigTest, BracesConfig) {
    EXPECT_TRUE(ParseString("server { listen 80; }"));
    EXPECT_EQ(1, out_config_.statements_.size());
    EXPECT_EQ("server", out_config_.statements_.at(0)->tokens_.at(0));

    EXPECT_EQ("listen", out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("80", out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(1));
}

TEST_F(NginxStringConfigTest, MultipleBracesConfig) {
    EXPECT_TRUE(ParseString("server { listen 80; } Node { int val; }"));
    EXPECT_EQ(2, out_config_.statements_.size());

    EXPECT_EQ("server", out_config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("listen", out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("80", out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(1));

    EXPECT_EQ("Node", out_config_.statements_.at(1)->tokens_.at(0));
    EXPECT_EQ("int", out_config_.statements_.at(1)->child_block_->statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("val", out_config_.statements_.at(1)->child_block_->statements_.at(0)->tokens_.at(1));


}

TEST_F(NginxStringConfigTest, NestedBracesConfig) {
    EXPECT_TRUE(ParseString("if (true) { if (false) { return false; } }"));
    EXPECT_EQ(1, out_config_.statements_.size());

    EXPECT_EQ("if", out_config_.statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("(true)", out_config_.statements_.at(0)->tokens_.at(1));
    EXPECT_EQ("if", out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("(false)", out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(1));

    EXPECT_EQ("return", out_config_.statements_.at(0)->child_block_->statements_.at(
        0)->child_block_->statements_.at(0)->tokens_.at(0));
    EXPECT_EQ("false", out_config_.statements_.at(0)->child_block_->statements_.at(
        0)->child_block_->statements_.at(0)->tokens_.at(1));
}

