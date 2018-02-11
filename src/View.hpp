﻿#pragma once

//
// 表示関連
//

#include <boost/noncopyable.hpp>
#include <cinder/TriMesh.h>
#include <cinder/gl/Vbo.h>
#include "PLY.hpp"


namespace ngs {

enum {
  PANEL_SIZE = 20,
};


struct View
  : private boost::noncopyable
{
  // パネル
  std::vector<std::string> panel_path;
  std::vector<ci::gl::VboMeshRef> panel_models;

  // 演出用
  ci::gl::VboMeshRef blank_model;
  ci::gl::VboMeshRef selected_model;
  ci::gl::VboMeshRef cursor_model;

  // 背景
  ci::gl::VboMeshRef bg_model; 

  // Field上のパネル(Modelと被っている)
  struct Panel
  {
    glm::vec3 position;
    glm::vec3 rotation;
    int index;
  };
  std::vector<Panel> field_panels_;


  View(const ci::JsonTree& params) noexcept
  {
    const auto& path = params["panel_path"];
    for (const auto p : path)
    {
      panel_path.push_back(p.getValue<std::string>());
    }
    panel_models.resize(panel_path.size());

    blank_model    = ci::gl::VboMesh::create(PLY::load(params.getValueForKey<std::string>("blank_model")));
    selected_model = ci::gl::VboMesh::create(PLY::load(params.getValueForKey<std::string>("selected_model")));
    cursor_model   = ci::gl::VboMesh::create(PLY::load(params.getValueForKey<std::string>("cursor_model")));
    bg_model       = ci::gl::VboMesh::create(PLY::load(params.getValueForKey<std::string>("bg_model")));
  }

  ~View() = default;


  // パネル追加
  void addPanel(int index, glm::ivec2 pos, u_int rotation) noexcept
  {
    glm::ivec2 p = pos * int(PANEL_SIZE);

    static const float r_tbl[] = {
      0.0f,
      -180.0f * 0.5f,
      -180.0f,
      -180.0f * 1.5f 
    };

    Panel panel = {
      { p.x, 0, p.y },
      { 0, ci::toRadians(r_tbl[rotation]), 0 }, 
      index
    };

    field_panels_.push_back(panel);
  }
};


// 読まれてないパネルを読み込む
const ci::gl::VboMeshRef& getPanelModel(View& view, const int number) noexcept
{
  if (!view.panel_models[number])
  {
    auto mesh = ci::gl::VboMesh::create(PLY::load(view.panel_path[number]));
    view.panel_models[number] = mesh;
  }

  return view.panel_models[number];
}




#ifdef DEBUG

// パネルのエッジを表示
void drawPanelEdge(const Panel& panel, glm::vec3 pos, u_int rotation) noexcept
{
  const float r_tbl[] = {
    0.0f,
    -180.0f * 0.5f,
    -180.0f,
    -180.0f * 1.5f 
  };

  ci::gl::pushModelView();
  ci::gl::translate(pos.x, pos.y, pos.z);
  ci::gl::rotate(toRadians(ci::vec3(0.0f, r_tbl[rotation], 0.0f)));

  ci::gl::lineWidth(10);

  const auto& edge = panel.getEdge();
  for (auto e : edge) {
    ci::Color col;
    if (e & Panel::PATH)   col = ci::Color(1.0, 1.0, 0.0);
    if (e & Panel::FOREST) col = ci::Color(0.0, 0.5, 0.0);
    if (e & Panel::GRASS)  col = ci::Color(0.0, 1.0, 0.0);
    ci::gl::color(col);

    ci::gl::drawLine(ci::vec3(-10.1, 1, 10.1), ci::vec3(10.1, 1, 10.1));
    ci::gl::rotate(toRadians(ci::vec3(0.0f, 90.0f, 0.0f)));
  }

  ci::gl::popModelView();

  ci::gl::color(ci::Color(1, 1, 1));
}

#endif

// パネルを１枚表示
void drawPanel(int number, glm::vec3 pos, u_int rotation, View& view, float rotate_offset) noexcept
{
  const float r_tbl[] = {
    0.0f,
    -180.0f * 0.5f,
    -180.0f,
    -180.0f * 1.5f 
  };

  ci::gl::pushModelView();
  ci::gl::translate(pos.x, pos.y, pos.z);
  ci::gl::rotate(toRadians(ci::vec3(0.0f, r_tbl[rotation] + rotate_offset, 0.0f)));
  const auto& model = getPanelModel(view, number);
  ci::gl::draw(model);
  ci::gl::popModelView();
}

void drawPanel(int number, glm::ivec2 pos, u_int rotation, View& view) noexcept
{
  drawPanel(number, glm::vec3(pos.x, 0.0f, pos.y), rotation, view, 0.0f);
}

// Fieldのパネルをすべて表示
void drawFieldPanels(const std::vector<PanelStatus>& panels, View& view) noexcept
{
  for (const auto& p : panels) {
    drawPanel(p.number, p.position * int(PANEL_SIZE), p.rotation, view);
  }
}

// NOTICE 中でViewが書き換わっているのでconstにできない
void drawFieldPanels(View& view) noexcept
{
  const auto& panels = view.field_panels_;
  for (const auto& p : panels)
  {
    ci::gl::pushModelView();
    ci::gl::translate(p.position);
    ci::gl::rotate(p.rotation);

    const auto& model = getPanelModel(view, p.index);
    ci::gl::draw(model);
    ci::gl::popModelView();
  }
}

// Fieldの置ける場所をすべて表示
void drawFieldBlank(const std::vector<glm::ivec2>& blank, View& view) noexcept
{
  for (const auto& pos : blank) {
    glm::ivec2 p = pos * int(PANEL_SIZE);

    ci::gl::pushModelView();
    ci::gl::translate(p.x, 0.0f, p.y);
    ci::gl::draw(view.blank_model);
    ci::gl::popModelView();
  }
}

// 置けそうな箇所をハイライト
void drawFieldSelected(glm::ivec2 pos, glm::vec3 scale, const View& view) noexcept
{
  glm::ivec2 p = pos * int(PANEL_SIZE);

  ci::gl::pushModelView();
  ci::gl::translate(p.x, 0.0f, p.y);
  ci::gl::scale(scale.x, scale.y, scale.z);
  ci::gl::draw(view.selected_model);
  ci::gl::popModelView();
}

void drawCursor(glm::vec3 pos, glm::vec3 scale, const View& view) noexcept
{
  ci::gl::pushModelView();
  ci::gl::translate(pos.x, pos.y, pos.z);
  ci::gl::scale(scale.x, scale.y, scale.z);
  ci::gl::draw(view.cursor_model);
  ci::gl::popModelView();
}


// 背景
void drawFieldBg(const View& view) noexcept
{
  ci::gl::pushModelView();
  ci::gl::translate(10, -15.0, 10);
  ci::gl::scale(20.0, 10.0, 20.0);
  ci::gl::draw(view.bg_model);
  ci::gl::popModelView();
}

}

