/* Copyright (C) 2019-2021 GSI, Universität Tübingen, MEPhI
   SPDX-License-Identifier: GPL-3.0-only
   Authors: Viktor Klochkov, Eugeny Kashirin, Ilya Selyuzhenkov */
#include "Track.hpp"

#include <TDatabasePDG.h>

#include <iostream>

namespace AnalysisTree {

void Track::Print() const noexcept {
  std::cout << " Px = " << px_ << "  Py = " << py_ << "  Pz = " << pz_
            << "  phi = " << GetPhi() << "  pT = " << GetPt() << "  eta = " << GetEta() << std::endl;
  Container::Print();
  if (!hit_map_.empty()) {
    std::cout << "Hit map: ";
    for (auto h : hit_map_) std::cout << h << " ";
    std::cout << std::endl;
  }
}

void Track::SetHitMap(const std::vector<bool>& hits) {
  hit_map_ = hits;
}

int Track::CountAllHits() const {
  return std::count(hit_map_.begin(), hit_map_.end(), true);
}

void Track::SetDetectorHits(ShortInt_t offset, ShortInt_t size, const std::vector<bool>& hits) {
  if (static_cast<ShortInt_t>(hits.size()) != size) {
    throw std::runtime_error("Track::SetDetectorHits(): size mismatch: expected " +
                             std::to_string(size) + ", received " + std::to_string(hits.size()));
  }
  if (static_cast<ShortInt_t>(hit_map_.size()) < offset + size) {
    hit_map_.resize(offset + size, false);
  }
  for (ShortInt_t i = 0; i < size; ++i) {
    hit_map_[offset + i] = hits[i];
  }
}

std::vector<bool> Track::GetDetectorHits(ShortInt_t offset, ShortInt_t size) const {
  if (static_cast<ShortInt_t>(hit_map_.size()) < offset + size) {
    throw std::out_of_range("Track::GetDetectorHits(): [" + std::to_string(offset) + ", " + 
		            std::to_string(offset + size) + ") exceeds hit_map_ size of " + 
			    std::to_string(hit_map_.size()));
  }
  return {hit_map_.begin() + offset, hit_map_.begin() + offset + size};
}

int Track::CountHits(ShortInt_t offset, ShortInt_t size) const {
  if (static_cast<ShortInt_t>(hit_map_.size()) < offset + size) {
    throw std::out_of_range("Track::CountHits(): [" + std::to_string(offset) + ", " +
		            std::to_string(offset + size) + ") exceeds hit_map_ size of " + 
			    std::to_string(hit_map_.size()));
  }
  return std::count(hit_map_.begin() + offset, hit_map_.begin() + offset + size, true);
}

bool operator==(const Track& that, const Track& other) noexcept {
  if (&that == &other) {
    return true;
  }
  if ((Container&) that != (Container&) other) {
    return false;
  }
  return that.px_ == other.px_ && that.py_ == other.py_ && that.pz_ == other.pz_;
}

float Track::GetMassByPdgId(PdgCode_t pdg) {

  if (pdg > 1000000000) {//100ZZZAAA0
    auto A = (pdg % 10000) / 10;
    return A * 0.938f /* GeV */;
  }
  auto db = TDatabasePDG::Instance();
  auto particle = db->GetParticle(pdg);

  if (particle) {
    return particle->Mass();
  } else {
    throw std::runtime_error("Mass of " + std::to_string(pdg) + " is not known");
  }
}

int Track::GetChargeByPdgId(PdgCode_t pdg) {

  if (pdg > 1000000000) {//100ZZZAAA0
    auto Z = (pdg % 10000000) / 10000;
    return Z;
  }
  auto db = TDatabasePDG::Instance();
  auto particle = db->GetParticle(pdg);

  if (particle) {
    return int(particle->Charge() / 3);
  } else {
    throw std::runtime_error("Mass of " + std::to_string(pdg) + " is not known");
  }
}

}// namespace AnalysisTree
