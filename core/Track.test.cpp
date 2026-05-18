/* Copyright (C) 2019-2021 GSI, Universität Tübingen
   SPDX-License-Identifier: GPL-3.0-only
   Authors: Viktor Klochkov, Ilya Selyuzhenkov */
#ifndef ANALYSISTREE_TEST_CORE_TRACK_TEST_HPP_
#define ANALYSISTREE_TEST_CORE_TRACK_TEST_HPP_

#include "TDatabasePDG.h"

#include <Math/Vector4D.h>
#include <TFile.h>
#include <TTree.h>

#include <gtest/gtest.h>
#include <vector>

#include <core/BranchConfig.hpp>
#include <core/Configuration.hpp>
#include <core/Detector.hpp>

namespace {

using namespace AnalysisTree;

TEST(Track, Basics) {

  Track track(0);

  const float px = std::rand() * (1. / RAND_MAX * 2.);
  const float py = std::rand() * (1. / RAND_MAX * 1.5);
  const float pz = 2. + std::rand() * (1. / RAND_MAX);

  ROOT::Math::PxPyPzMVector vec(px, py, pz, 0.13957);

  track.SetMomentum(px, py, pz);

  ASSERT_FLOAT_EQ(track.GetPx(), px);
  ASSERT_FLOAT_EQ(track.GetPy(), py);
  ASSERT_FLOAT_EQ(track.GetPz(), pz);
  ASSERT_FLOAT_EQ(track.GetPt(), sqrt(px * px + py * py));
  ASSERT_FLOAT_EQ(track.GetPhi(), float(TMath::ATan2(py, px)));
  ASSERT_FLOAT_EQ(track.GetP(), sqrt(px * px + py * py + pz * pz));
  ASSERT_NEAR(track.GetRapidity(211), vec.Rapidity(), 1e-5);

  //  auto vec1 = track.Get4Momentum(211);
  //
  //  ASSERT_NEAR(vec1.Rapidity(), vec.Rapidity(), 1e-5);
  //  ASSERT_NEAR(vec1.M(), vec.M(), 1e-5);
}

TEST(Track, Write) {

  TFile* f = TFile::Open("test.root", "recreate");
  TTree* t{new TTree("test", "")};

  Configuration config;

  BranchConfig RecTracksBranch("RecTrack", DetType::kTrack);
  RecTracksBranch.AddField<float>("dcax", "cm");
  RecTracksBranch.AddField<float>("dcay", "cm");
  RecTracksBranch.AddField<float>("dcaz", "cm");
  RecTracksBranch.AddField<int>("nhits", "Number of hits");

  config.AddBranchConfig(RecTracksBranch);
  auto* RecTracks = new TrackDetector(0);

  t->Branch("RecTracks", "AnalysisTree::TrackDetector", &RecTracks);

  for (int i = 0; i < 10; ++i) {
    RecTracks->ClearChannels();
    int n_tracks = 5;//std::rand() % 100;
    for (int j = 0; j < n_tracks; ++j) {
      auto* iTrack = RecTracks->AddChannel();
      iTrack->Init(RecTracksBranch);

      const float px = std::rand() * (1. / RAND_MAX * 2.);
      const float py = std::rand() * (1. / RAND_MAX * 1.5);
      const float pz = 2. + std::rand() * (1. / RAND_MAX);

      iTrack->SetMomentum(px, py, pz);
      iTrack->SetField(1.f, RecTracksBranch.GetFieldId("dcax"));
      iTrack->SetField(2.f, RecTracksBranch.GetFieldId("dcay"));
      iTrack->SetField(3.f, RecTracksBranch.GetFieldId("dcaz"));
      iTrack->SetField(4, RecTracksBranch.GetFieldId("nhits"));
    }
    t->Fill();
  }

  config.Write("Configuration");
  t->Write();
  f->Close();
}

TEST(Track, HitMap) {
  BranchConfig branch_config("RecTracks", DetType::kTrack);
  branch_config.AddDetectorToHitMap("Det0",  4);
  branch_config.AddDetectorToHitMap("Det1", 12);
  branch_config.AddDetectorToHitMap("Det2",  1);

  const auto off0 = branch_config.GetHitMapOffset("Det0");
  const auto sz0  = branch_config.GetHitMapSize("Det0");
  const auto off1 = branch_config.GetHitMapOffset("Det1");
  const auto sz1  = branch_config.GetHitMapSize("Det1");
  const auto off2 = branch_config.GetHitMapOffset("Det2");
  const auto sz2  = branch_config.GetHitMapSize("Det2");

  Track track(0);

  // Fill each detector slice; SetDetectorHits grows hit_map_ as needed
  track.SetDetectorHits(off0, sz0, {1, 1, 1, 0});
  track.SetDetectorHits(off1, sz1, {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0});
  track.SetDetectorHits(off2, sz2, {1});

  EXPECT_EQ(static_cast<int>(track.GetHitMap().size()), branch_config.GetTotalHitMapSize());

  // Count hits per detector
  EXPECT_EQ(track.CountHits(off0, sz0), 3);
  EXPECT_EQ(track.CountHits(off1, sz1), 6);
  EXPECT_EQ(track.CountHits(off2, sz2), 1);
  EXPECT_EQ(track.CountAllHits(), 10);

  // Retrieve a detector slice
  const auto det0_hits = track.GetDetectorHits(off0, sz0);
  ASSERT_EQ(det0_hits.size(), 4u);
  EXPECT_EQ(det0_hits[0], true);
  EXPECT_EQ(det0_hits[3], false);

  // SetHitMap replaces the entire vector at once
  track.SetHitMap(std::vector<bool>(branch_config.GetTotalHitMapSize(), true));
  EXPECT_EQ(track.CountAllHits(), branch_config.GetTotalHitMapSize());

  // Wrong size in SetDetectorHits must throw
  EXPECT_THROW(track.SetDetectorHits(off0, sz0, {1, 0}), std::runtime_error);

  // Out-of-range access in GetDetectorHits must throw
  EXPECT_THROW(track.GetDetectorHits(100, 5), std::out_of_range);
}

}// namespace

#endif//ANALYSISTREE_TEST_CORE_TRACK_TEST_HPP_
