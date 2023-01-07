/*
 * Covariant Script Regex Extension
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (C) 2017-2023 Michael Lee(李登淳)
 *
 * Email:   lee@covariant.cn, mikecovlee@163.com
 * Github:  https://github.com/mikecovlee
 * Website: http://covscript.org.cn
 */
#include <covscript/cni.hpp>
#include <covscript/dll.hpp>
#include <regex>

static cs::namespace_t regex_ext = cs::make_shared_namespace<cs::name_space>();
static cs::namespace_t regex_result_ext =
    cs::make_shared_namespace<cs::name_space>();

namespace cs_impl {
	template <>
	cs::namespace_t &get_ext<std::regex>()
	{
		return regex_ext;
	}

	template <>
	cs::namespace_t &get_ext<std::smatch>()
	{
		return regex_result_ext;
	}

	template <>
	constexpr const char *get_name_of_type<std::regex>()
	{
		return "cs::regex";
	}

	template <>
	constexpr const char *get_name_of_type<std::smatch>()
	{
		return "cs::regex::result";
	}
}  // namespace cs_impl
namespace regex_cs_ext {
	using namespace cs;

	var build(const string &str)
	{
		return var::make<std::regex>(str);
	}

	std::smatch match(std::regex &reg, const string &str)
	{
		std::smatch m;
		std::regex_search(str, m, reg);
		return std::move(m);
	}

	std::smatch search(std::regex &reg, const string &str)
	{
		std::smatch m;
		std::regex_search(str, m, reg);
		return std::move(m);
	}

	string replace(std::regex &reg, const string &str, const string &fmt)
	{
		return std::regex_replace(str, reg, fmt);
	}

	bool ready(const std::smatch &m)
	{
		return m.ready();
	}

	bool empty(const std::smatch &m)
	{
		return m.empty();
	}

	numeric size(const std::smatch &m)
	{
		return m.size();
	}

	numeric length(const std::smatch &m, numeric index)
	{
		return m.length(index.as_integer());
	}

	numeric position(const std::smatch &m, numeric index)
	{
		return m.position(index.as_integer());
	}

	string str(const std::smatch &m, numeric index)
	{
		return m.str(index.as_integer());
	}

	string prefix(const std::smatch &m)
	{
		return m.prefix().str();
	}

	string suffix(const std::smatch &m)
	{
		return m.suffix().str();
	}

	void init(name_space *ns)
	{
		(*ns)
		.add_var("result", make_namespace(regex_result_ext))
		.add_var("build", make_cni(build))
		.add_var("match", make_cni(match))
		.add_var("search", make_cni(search))
		.add_var("replace", make_cni(replace));
		(*regex_ext)
		.add_var("match", make_cni(match))
		.add_var("search", make_cni(search))
		.add_var("replace", make_cni(replace));
		(*regex_result_ext)
		.add_var("ready", make_cni(ready))
		.add_var("empty", make_cni(empty))
		.add_var("size", make_cni(size))
		.add_var("length", make_cni(length))
		.add_var("position", make_cni(position))
		.add_var("str", make_cni(str))
		.add_var("prefix", make_cni(prefix))
		.add_var("suffix", make_cni(suffix));
	}
}  // namespace regex_cs_ext

void cs_extension_main(cs::name_space *ns)
{
	regex_cs_ext::init(ns);
}