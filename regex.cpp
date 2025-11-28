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

#include "pcre2.hpp"

static cs::namespace_t regex_ext = cs::make_shared_namespace<cs::name_space>();
static cs::namespace_t regex_result_ext = cs::make_shared_namespace<cs::name_space>();

namespace cs_impl {
	template <>
	cs::namespace_t &get_ext<pcre2_regex_t>()
	{
		return regex_ext;
	}

	template <>
	cs::namespace_t &get_ext<pcre2_smatch>()
	{
		return regex_result_ext;
	}

	template <>
	constexpr const char *get_name_of_type<pcre2_regex_t>()
	{
		return "cs::regex";
	}

	template <>
	constexpr const char *get_name_of_type<pcre2_smatch>()
	{
		return "cs::regex::result";
	}
} // namespace cs_impl

namespace regex_cs_ext {
	using namespace cs;

	pcre2_regex_t build(const string &str)
	{
		return std::make_shared<pcre2_regex>(str, false);
	}

	pcre2_regex_t build_optimize(const string &str)
	{
		return std::make_shared<pcre2_regex>(str, true);
	}

	pcre2_smatch match(pcre2_regex_t &reg, const string &str)
	{
		return pcre2_regex_match(reg, str, PCRE2_ANCHORED | PCRE2_ENDANCHORED);
	}

	pcre2_smatch search(pcre2_regex_t &reg, const string &str)
	{
		return pcre2_regex_match(reg, str, 0);
	}

	string replace(pcre2_regex_t &reg, const string &str, const string &fmt)
	{
		return pcre2_regex_replace(reg, str, fmt);
	}

	bool ready(const pcre2_smatch &m)
	{
		return m.ready;
	}

	bool empty(const pcre2_smatch &m)
	{
		return m.empty();
	}

	numeric size(const pcre2_smatch &m)
	{
		return m.size();
	}

	numeric length(const pcre2_smatch &m, numeric index)
	{
		return m.str(index.as_integer()).size();
	}

	numeric position(const pcre2_smatch &m, numeric index)
	{
		return m.position(index.as_integer());
	}

	string str(const pcre2_smatch &m, numeric index)
	{
		return string(m.str(index.as_integer()));
	}

	string prefix(const pcre2_smatch &m)
	{
		return m.prefix();
	}

	string suffix(const pcre2_smatch &m)
	{
		return m.suffix();
	}

	void init(name_space *ns)
	{
		(*ns)
		.add_var("result", make_namespace(regex_result_ext))
		.add_var("build", make_cni(build))
		.add_var("build_optimize", make_cni(build_optimize))
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
} // namespace regex_cs_ext

void cs_extension_main(cs::name_space *ns)
{
	regex_cs_ext::init(ns);
}