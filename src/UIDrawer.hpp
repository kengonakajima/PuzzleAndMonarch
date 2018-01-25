﻿#pragma once

//
// UI描画色々
//

#include <cinder/Json.h>
#include <cinder/gl/Shader.h>
#include "Font.hpp"
#include "Shader.hpp"
#include <map>


namespace ngs { namespace UI {

struct Drawer
{
  Drawer(const ci::JsonTree& params) noexcept
  {
    const auto& json = params["font"];

    for (size_t i = 0; i < json.getNumChildren(); ++i)
    {
      const auto& p = json[i];

      const auto& name = p.getValueForKey<std::string>("name");
      const auto& path = p.getValueForKey<std::string>("path");
      const int size   = p.getValueForKey<int>("size");

      fonts_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(name),
                     std::forward_as_tuple(path, size));
    }

    {
      auto name = params.getValueForKey<std::string>("shader.font");
      font_shader_ = createShader(name, name);
    }
    {
      auto color = ci::gl::ShaderDef().color();
      color_shader_ = ci::gl::getStockShader(color);
    }
  }


  Font& getFont(const std::string& name) noexcept
  {
    return fonts_.at(name);
  }

  const ci::gl::GlslProgRef& getFontShader() const noexcept
  {
    return font_shader_;
  }

  const ci::gl::GlslProgRef& getColorShader() const noexcept
  {
    return color_shader_;
  }


private:
  std::map<std::string, Font> fonts_;

  ci::gl::GlslProgRef font_shader_;
  ci::gl::GlslProgRef color_shader_;

};

} }
