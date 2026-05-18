/* Copyright (C) 2019-2021 GSI, Universität Tübingen
   SPDX-License-Identifier: GPL-3.0-only
   Authors: Viktor Klochkov, Ilya Selyuzhenkov */
#ifndef ANALYSISTREE_CORE_BRANCHCONFIG_TEST_H_
#define ANALYSISTREE_CORE_BRANCHCONFIG_TEST_H_

#include <gtest/gtest.h>

#include "BranchConfig.hpp"

namespace {

using namespace AnalysisTree;

TEST(BranchConfig, Basics) {

  auto det_types = {DetType::kTrack, DetType::kModule, DetType::kParticle, DetType::kHit, DetType::kEventHeader};
  for (auto det_type : det_types) {
    BranchConfig branch_config("RecTrack", det_type);
    branch_config.AddField<float>("test_f", "just a test field");
    branch_config.AddField<int>("test_i", "just a test field");
    branch_config.AddField<bool>("test_b", "just a test field");

    EXPECT_EQ(branch_config.GetFieldId("test_i"), 0);
    EXPECT_EQ(branch_config.GetFieldId("test_b"), 0);
    EXPECT_EQ(branch_config.GetFieldId("test_f"), 0);

    EXPECT_EQ(branch_config.GetFieldType("test_f"), Types::kFloat);
    EXPECT_EQ(branch_config.GetFieldType("test_i"), Types::kInteger);
    EXPECT_EQ(branch_config.GetFieldType("test_b"), Types::kBool);
  }

  BranchConfig branch_config("RecTrack", DetType::kTrack);

  EXPECT_EQ(branch_config.GetFieldId("pT"), TrackFields::kPt);
  EXPECT_EQ(branch_config.GetFieldId("phi"), TrackFields::kPhi);
  EXPECT_EQ(branch_config.GetFieldId("eta"), TrackFields::kEta);
  EXPECT_EQ(branch_config.GetFieldId("p"), TrackFields::kP);
  EXPECT_EQ(branch_config.GetFieldId("px"), TrackFields::kPx);
  EXPECT_EQ(branch_config.GetFieldId("py"), TrackFields::kPy);
  EXPECT_EQ(branch_config.GetFieldId("pz"), TrackFields::kPz);
}

TEST(BranchConfig, HitMap) {
  BranchConfig branch_config("RecTracks", DetType::kTrack);
  branch_config.AddDetectorToHitMap("Det0",  4);
  branch_config.AddDetectorToHitMap("Det1", 12);
  branch_config.AddDetectorToHitMap("Det2",  1);

  // Total size is the sum of all station counts
  EXPECT_EQ(branch_config.GetTotalHitMapSize(), 17);

  // Offsets are cumulative sums: Det0 starts at 0, Det1 at 4, Det2 at 16
  EXPECT_EQ(branch_config.GetHitMapOffset("Det0"),  0);
  EXPECT_EQ(branch_config.GetHitMapOffset("Det1"),  4);
  EXPECT_EQ(branch_config.GetHitMapOffset("Det2"), 16);

  // Sizes match the registered station counts
  EXPECT_EQ(branch_config.GetHitMapSize("Det0"),  4);
  EXPECT_EQ(branch_config.GetHitMapSize("Det1"), 12);
  EXPECT_EQ(branch_config.GetHitMapSize("Det2"),  1);

  EXPECT_TRUE(branch_config.HasHitMap("Det0"));
  EXPECT_FALSE(branch_config.HasHitMap("Det3"));

  // Registering the same detector twice must throw
  EXPECT_THROW(branch_config.AddDetectorToHitMap("Det0", 4), std::runtime_error);

  // Detectors are returned in registration order
  const auto& dets = branch_config.GetHitMapDetectors();
  ASSERT_EQ(dets.size(), 3u);
  EXPECT_EQ(dets[0], "Det0");
  EXPECT_EQ(dets[1], "Det1");
  EXPECT_EQ(dets[2], "Det2");

  // Unknown detector returns UndefValueShort
  EXPECT_EQ(branch_config.GetHitMapOffset("UNKNOWN"), UndefValueShort);
  EXPECT_EQ(branch_config.GetHitMapSize("UNKNOWN"),   UndefValueShort);

  // Clone must carry the hit map configuration unchanged
  auto cloned = branch_config.Clone("RecTracks2", DetType::kTrack);
  EXPECT_EQ(cloned.GetTotalHitMapSize(), 17);
  EXPECT_EQ(cloned.GetHitMapOffset("Det1"), 4);
}

}// namespace

#endif//ANALYSISTREE_CORE_BRANCHCONFIG_TEST_H_
