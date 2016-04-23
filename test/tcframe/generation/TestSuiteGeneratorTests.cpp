#include "gmock/gmock.h"

#include "../os/MockOperatingSystem.hpp"
#include "FakeTestCaseGenerationFailure.hpp"
#include "MockTestCaseGenerator.hpp"
#include "MockTestSuiteGenerationListener.hpp"
#include "tcframe/generation/TestSuiteGenerator.hpp"

using ::testing::_;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Pair;
using ::testing::Property;
using ::testing::Return;
using ::testing::Test;

namespace tcframe {

class TestSuiteGeneratorTests : public Test {
protected:
    NiceMock<MockTestCaseGenerator> testCaseGenerator;
    NiceMock<MockOperatingSystem> os;
    NiceMock<MockTestSuiteGenerationListener> generationListener;

    TestCase tc1 = TestCase([]{}, "N = 1");
    TestCase tc2 = TestCase([]{}, "N = 2");
    TestCase tc3 = TestCase([]{}, "N = 3");

    TestSuite testSuiteWithoutGroups = TestSuiteBuilder()
            .addTestCase(tc1)
            .addTestCase(tc2)
            .build();
    TestSuite testSuiteWithGroups = TestSuiteBuilder()
            .newTestGroup()
            .setConstraintGroupIds({1, 2})
            .addTestCase(tc1)
            .addTestCase(tc2)
            .newTestGroup()
            .setConstraintGroupIds({2})
            .addTestCase(tc3)
            .build();

    ProblemConfig problemConfig = ProblemConfigBuilder()
            .setSlug("foo")
            .build();
    GeneratorConfig generatorConfig = GeneratorConfigBuilder()
            .setSolutionCommand("python Sol.py")
            .setTestCasesDir("dir")
            .build();

    TestSuiteGenerator generator = TestSuiteGenerator(&testCaseGenerator, &os, &generationListener);

    void SetUp() {
        ON_CALL(testCaseGenerator, generate(_, _, generatorConfig))
                .WillByDefault(Return(TestCaseGenerationResult::successfulResult()));
    }
};

TEST_F(TestSuiteGeneratorTests, WithoutGroups_SuccessfulGeneration) {
    {
        InSequence sequence;
        EXPECT_CALL(generationListener, onIntroduction());
        EXPECT_CALL(os, forceMakeDir("dir"));

        EXPECT_CALL(generationListener, onTestGroupIntroduction(-1));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_1"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_1").setDescription("N = 1").setConstraintGroupIds({-1}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 1", TestCaseGenerationResult::successfulResult()));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_2"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_2").setDescription("N = 2").setConstraintGroupIds({-1}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 2", TestCaseGenerationResult::successfulResult()));
    }

    TestSuiteGenerationResult result = generator.generate(testSuiteWithoutGroups, problemConfig, generatorConfig);

    EXPECT_THAT(result.isSuccessful(), Eq(true));
    EXPECT_THAT(result.testCaseGenerationResultByTestCaseName(), ElementsAre(
            Pair("foo_1", TestCaseGenerationResult::successfulResult()),
            Pair("foo_2", TestCaseGenerationResult::successfulResult())));
}

TEST_F(TestSuiteGeneratorTests, WithoutGroups_UnsuccessfulGeneration) {
    TestCaseGenerationFailure* failure_2 = new FakeTestCaseGenerationFailure();
    ON_CALL(testCaseGenerator, generate(Property(&TestCaseData::name, "foo_2"), _, generatorConfig))
            .WillByDefault(Return(TestCaseGenerationResult::failedResult(failure_2)));
    {
        InSequence sequence;
        EXPECT_CALL(generationListener, onIntroduction());
        EXPECT_CALL(os, forceMakeDir("dir"));

        EXPECT_CALL(generationListener, onTestGroupIntroduction(-1));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_1"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_1").setDescription("N = 1").setConstraintGroupIds({-1}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 1", TestCaseGenerationResult::successfulResult()));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_2"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_2").setDescription("N = 2").setConstraintGroupIds({-1}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 2", TestCaseGenerationResult::failedResult(failure_2)));
    }

    TestSuiteGenerationResult result = generator.generate(testSuiteWithoutGroups, problemConfig, generatorConfig);

    EXPECT_THAT(result.isSuccessful(), Eq(false));
    EXPECT_THAT(result.testCaseGenerationResultByTestCaseName(), ElementsAre(
            Pair("foo_1", TestCaseGenerationResult::successfulResult()),
            Pair("foo_2", TestCaseGenerationResult::failedResult(failure_2))));
}

TEST_F(TestSuiteGeneratorTests, WithGroups_SuccessfulGeneration) {
    {
        InSequence sequence;
        EXPECT_CALL(generationListener, onIntroduction());
        EXPECT_CALL(os, forceMakeDir("dir"));

        EXPECT_CALL(generationListener, onTestGroupIntroduction(1));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_1_1"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_1_1").setDescription("N = 1").setConstraintGroupIds({1, 2}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 1", TestCaseGenerationResult::successfulResult()));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_1_2"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_1_2").setDescription("N = 2").setConstraintGroupIds({1, 2}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 2", TestCaseGenerationResult::successfulResult()));


        EXPECT_CALL(generationListener, onTestGroupIntroduction(2));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_2_1"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_2_1").setDescription("N = 3").setConstraintGroupIds({2}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 3", TestCaseGenerationResult::successfulResult()));
    }

    TestSuiteGenerationResult result = generator.generate(testSuiteWithGroups, problemConfig, generatorConfig);

    EXPECT_THAT(result.isSuccessful(), Eq(true));
    EXPECT_THAT(result.testCaseGenerationResultByTestCaseName(), ElementsAre(
            Pair("foo_1_1", TestCaseGenerationResult::successfulResult()),
            Pair("foo_1_2", TestCaseGenerationResult::successfulResult()),
            Pair("foo_2_1", TestCaseGenerationResult::successfulResult())));
}

TEST_F(TestSuiteGeneratorTests, WithGroups_UnsuccessfulGeneration) {
    TestCaseGenerationFailure* failure_1_2 = new FakeTestCaseGenerationFailure();
    ON_CALL(testCaseGenerator, generate(Property(&TestCaseData::name, "foo_1_2"), _, generatorConfig))
            .WillByDefault(Return(TestCaseGenerationResult::failedResult(failure_1_2)));
    TestCaseGenerationFailure* failure_2_1 = new FakeTestCaseGenerationFailure();
    ON_CALL(testCaseGenerator, generate(Property(&TestCaseData::name, "foo_2_1"), _, generatorConfig))
            .WillByDefault(Return(TestCaseGenerationResult::failedResult(failure_2_1)));
    {
        InSequence sequence;
        EXPECT_CALL(generationListener, onIntroduction());
        EXPECT_CALL(os, forceMakeDir("dir"));

        EXPECT_CALL(generationListener, onTestGroupIntroduction(1));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_1_1"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_1_1").setDescription("N = 1").setConstraintGroupIds({1, 2}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 1", TestCaseGenerationResult::successfulResult()));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_1_2"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_1_2").setDescription("N = 2").setConstraintGroupIds({1, 2}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 2", TestCaseGenerationResult::failedResult(failure_1_2)));


        EXPECT_CALL(generationListener, onTestGroupIntroduction(2));

        EXPECT_CALL(generationListener, onTestCaseIntroduction("foo_2_1"));
        EXPECT_CALL(testCaseGenerator, generate(
                TestCaseDataBuilder().setName("foo_2_1").setDescription("N = 3").setConstraintGroupIds({2}).build(),
                _,
                generatorConfig));
        EXPECT_CALL(generationListener, onTestCaseGenerationResult("N = 3", TestCaseGenerationResult::failedResult(failure_2_1)));
    }

    TestSuiteGenerationResult result = generator.generate(testSuiteWithGroups, problemConfig, generatorConfig);

    EXPECT_THAT(result.isSuccessful(), Eq(false));
    EXPECT_THAT(result.testCaseGenerationResultByTestCaseName(), ElementsAre(
            Pair("foo_1_1", TestCaseGenerationResult::successfulResult()),
            Pair("foo_1_2", TestCaseGenerationResult::failedResult(failure_1_2)),
            Pair("foo_2_1", TestCaseGenerationResult::failedResult(failure_2_1))));
}

}
