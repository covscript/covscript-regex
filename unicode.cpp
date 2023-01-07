/*
* Covariant Script Unicode Extension
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

#include <covscript/dll.hpp>
#include <covscript/cni.hpp>
#include <codecvt>
#include <cwctype>
#include <regex>

namespace codecvt_impl {
	using namespace cs;

	class charset {
	public:
		virtual ~charset() = default;

		virtual std::u32string local2wide(const std::string &) = 0;

		virtual std::string wide2local(const std::u32string &) = 0;

		virtual bool is_identifier(char32_t) = 0;
	};

	class ascii final : public charset {
	public:
		std::u32string local2wide(const std::string &local) override
		{
			return std::u32string(local.begin(), local.end());
		}

		std::string wide2local(const std::u32string &str) override
		{
			std::string local;
			local.reserve(str.size());
			for (auto ch:str)
				local.push_back(ch);
			return std::move(local);
		}

		bool is_identifier(char32_t ch) override
		{
			return ch == '_' || std::iswalnum(ch);
		}
	};

	class utf8 final : public charset {
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;

		static constexpr std::uint32_t ascii_max = 0x7F;
	public:
		std::u32string local2wide(const std::string &str) override
		{
			return cvt.from_bytes(str);
		}

		std::string wide2local(const std::u32string &str) override
		{
			return cvt.to_bytes(str);
		}

		bool is_identifier(char32_t ch) override
		{
			/**
			 * Chinese Character in Unicode Charset
			 * Basic:    0x4E00 - 0x9FA5
			 * Extended: 0x9FA6 - 0x9FEF
			 * Special:  0x3007
			 */
			if (ch > ascii_max)
				return (ch >= 0x4E00 && ch <= 0x9FA5) || (ch >= 0x9FA6 && ch <= 0x9FEF) || ch == 0x3007;
			else
				return ch == '_' || std::iswalnum(ch);
		}
	};

	class gbk final : public charset {
		static inline char32_t set_zero(char32_t ch)
		{
			return ch & 0x0000ffff;
		}

		static constexpr std::uint8_t u8_blck_begin = 0x80;
		static constexpr std::uint32_t u32_blck_begin = 0x8000;
	public:
		std::u32string local2wide(const std::string &local) override
		{
			std::u32string wide;
			uint32_t head = 0;
			bool read_next = true;
			for (auto it = local.begin(); it != local.end();) {
				if (read_next) {
					head = *(it++);
					if (head & u8_blck_begin)
						read_next = false;
					else
						wide.push_back(set_zero(head));
				}
				else {
					std::uint8_t tail = *(it++);
					wide.push_back(set_zero(head << 8 | tail));
					read_next = true;
				}
			}
			if (!read_next)
				throw compile_error("Codecvt: Bad encoding.");
			return std::move(wide);
		}

		std::string wide2local(const std::u32string &wide) override
		{
			std::string local;
			for (auto &ch:wide) {
				if (ch & u32_blck_begin)
					local.push_back(ch >> 8);
				local.push_back(ch);
			}
			return std::move(local);
		}

		bool is_identifier(char32_t ch) override
		{
			/**
			 * Chinese Character in GBK Charset
			 * GBK/2: 0xB0A1 - 0xF7FE
			 * GBK/3: 0x8140 - 0xA0FE
			 * GBK/4: 0xAA40 - 0xFEA0
			 * GBK/5: 0xA996
			 */
			if (ch & u32_blck_begin)
				return (ch >= 0xB0A1 && ch <= 0xF7FE) || (ch >= 0x8140 && ch <= 0xA0FE) ||
				       (ch >= 0xAA40 && ch <= 0xFEA0) || ch == 0xA996;
			else
				return ch == '_' || std::iswalnum(ch);
		}
	};
}

using codecvt_type = std::shared_ptr<codecvt_impl::charset>;
using wregex_type = std::basic_regex<char32_t>;
using wsmatch_type = std::match_results<std::u32string::const_iterator>;

CNI_ROOT_NAMESPACE {
	using namespace cs;

	CNI_NAMESPACE(codecvt)
	{
		cs::var make_codecvt_ascii() {
			return codecvt_type(new codecvt_impl::ascii);
		}

		CNI_REGISTER(ascii, cs::var::make_constant<cs::type_t>(make_codecvt_ascii, cs::type_id(typeid(codecvt_type))))

		cs::var make_codecvt_utf8() {
			return codecvt_type(new codecvt_impl::utf8);
		}

		CNI_REGISTER(utf8, cs::var::make_constant<cs::type_t>(make_codecvt_utf8, cs::type_id(typeid(codecvt_type))))

		cs::var make_codecvt_gbk() {
			return codecvt_type(new codecvt_impl::gbk);
		}

		CNI_REGISTER(gbk, cs::var::make_constant<cs::type_t>(make_codecvt_gbk, cs::type_id(typeid(codecvt_type))))

		std::u32string local2wide(const codecvt_type& cvt, const std::string& str) {
			return cvt->local2wide(str);
		}

		CNI(local2wide)

		std::string wide2local(const codecvt_type& cvt, const std::u32string& str) {
			return cvt->wide2local(str);
		}

		CNI(wide2local)

		bool is_identifier(const codecvt_type& cvt, char32_t ch) {
			return cvt->is_identifier(ch);
		}

		CNI(is_identifier)
	}

	CNI_NAMESPACE(wchar)
	{
		bool isalnum(char32_t c) {
			return std::iswalnum(c);
		}

		CNI(isalnum)

		bool isalpha(char32_t c) {
			return std::iswalpha(c);
		}

		CNI(isalpha)

		bool islower(char32_t c) {
			return std::iswlower(c);
		}

		CNI(islower)

		bool isupper(char32_t c) {
			return std::iswupper(c);
		}

		CNI(isupper)

		bool isdigit(char32_t c) {
			return std::iswdigit(c);
		}

		CNI(isdigit)

		bool iscntrl(char32_t c) {
			return std::iswcntrl(c);
		}

		CNI(iscntrl)

		bool isgraph(char32_t c) {
			return std::iswgraph(c);
		}

		CNI(isgraph)

		bool isspace(char32_t c) {
			return std::iswspace(c);
		}

		CNI(isspace)

		bool isblank(char32_t c) {
			return std::iswblank(c);
		}

		CNI(isblank)

		bool isprint(char32_t c) {
			return std::iswprint(c);
		}

		CNI(isprint)

		bool ispunct(char32_t c) {
			return std::iswpunct(c);
		}

		CNI(ispunct)

		char32_t tolower(char32_t c) {
			return std::towlower(c);
		}

		CNI(tolower)

		char32_t toupper(char32_t c) {
			return std::towupper(c);
		}

		CNI(toupper)

		char32_t from_unicode(const numeric& unicode) {
			if (unicode.as_integer() < 0)
				throw lang_error("Out of range.");
			return static_cast<char32_t>(unicode.as_integer());
		}

		CNI(from_unicode)

		std::u32string to_wstring(char32_t c) {
			return std::u32string(1, c);
		}

		CNI(to_wstring)
	}

	CNI_NAMESPACE(wstring_type)
	{
		char32_t at(const std::u32string &str, numeric idx) {
			return str.at(idx.as_integer());
		}

		CNI(at)

		std::u32string assign(std::u32string &str, numeric posit, char ch) {
			str.at(posit.as_integer()) = ch;
			return str;
		}

		CNI(assign)

		std::u32string append(std::u32string &str, const std::u32string &val) {
			str.append(val);
			return str;
		}

		CNI(append)

		std::u32string insert(std::u32string &str, numeric posit, const std::u32string &val) {
			str.insert(posit.as_integer(), val);
			return str;
		}

		CNI(insert)

		std::u32string erase(std::u32string &str, numeric b, numeric e) {
			str.erase(b.as_integer(), e.as_integer());
			return str;
		}

		CNI(erase)

		std::u32string replace(std::u32string &str, numeric posit, numeric count, const std::u32string &val) {
			str.replace(posit.as_integer(), count.as_integer(), val);
			return str;
		}

		CNI(replace)

		std::u32string substr(const std::u32string &str, numeric b, numeric e) {
			return str.substr(b.as_integer(), e.as_integer());
		}

		CNI(substr)

		numeric find(const std::u32string &str, const std::u32string &s, numeric posit) {
			auto pos = str.find(s, posit.as_integer());
			if (pos == std::u32string::npos)
				return -1;
			else
				return pos;
		}

		CNI(find)

		numeric rfind(const std::u32string &str, const std::u32string &s, numeric posit) {
			std::size_t pos = 0;
			if (posit.as_integer() == -1)
				pos = str.rfind(s, std::u32string::npos);
			else
				pos = str.rfind(s, posit.as_integer());
			if (pos == std::u32string::npos)
				return -1;
			else
				return pos;
		}

		CNI(rfind)

		std::u32string cut(std::u32string &str, numeric n) {
			for (std::size_t i = 0; i < n.as_integer(); ++i)
				str.pop_back();
			return str;
		}

		CNI(cut)

		bool empty(const std::u32string &str) {
			return str.empty();
		}

		CNI(empty)

		void clear(std::u32string &str) {
			str.clear();
		}

		CNI(clear)

		numeric size(const std::u32string &str) {
			return str.size();
		}

		CNI(size)

		std::u32string tolower(const std::u32string &str) {
			std::u32string s;
			for (auto &ch:str)
				s.push_back(std::towlower(ch));
			return std::move(s);
		}

		CNI(tolower)

		std::u32string toupper(const std::u32string &str) {
			std::u32string s;
			for (auto &ch:str)
				s.push_back(std::towupper(ch));
			return std::move(s);
		}

		CNI(toupper)

		numeric to_number(const std::u32string &str, const codecvt_type &cvt) {
			return parse_number(cvt->wide2local(str));
		}

		CNI(to_number)

		array split(const std::u32string &str, const array &signals) {
			array
			arr;
			std::u32string buf;
			bool found = false;
			for (auto &ch:str) {
				for (auto &sig:signals) {
					if (sig.type() == typeid(char) && ch == sig.const_val<char>()) {
						if (!buf.empty()) {
							arr.push_back(buf);
							buf.clear();
						}
						found = true;
						break;
					}
					else if (sig.type() == typeid(char32_t) && ch == sig.const_val<char32_t>()) {
						if (!buf.empty()) {
							arr.push_back(buf);
							buf.clear();
						}
						found = true;
						break;
					}
				}
				if (found)
					found = false;
				else
					buf.push_back(ch);
			}
			if (!buf.empty())
				arr.push_back(buf);
			return std::move(arr);
		}

		CNI(split)
	}

	CNI_NAMESPACE(wregex)
	{
		wsmatch_type match(wregex_type &reg, const std::u32string &str) {
			wsmatch_type m;
			std::regex_search(str, m, reg);
			return std::move(m);
		}

		wsmatch_type search(wregex_type &reg, const std::u32string &str) {
			wsmatch_type m;
			std::regex_search(str, m, reg);
			return std::move(m);
		}

		std::u32string replace(wregex_type &reg, const std::u32string &str, const std::u32string &fmt) {
			return std::regex_replace(str, reg, fmt);
		}
	}

	CNI_NAMESPACE(wsmatch)
	{
		bool ready(const wsmatch_type &m) {
			return m.ready();
		}

		bool empty(const wsmatch_type &m) {
			return m.empty();
		}

		numeric size(const wsmatch_type &m) {
			return m.size();
		}

		numeric length(const wsmatch_type &m, numeric index) {
			return m.length(index.as_integer());
		}

		numeric position(const wsmatch_type &m, numeric index) {
			return m.position(index.as_integer());
		}

		std::u32string str(const wsmatch_type &m, numeric index) {
			return m.str(index.as_integer());
		}

		std::u32string prefix(const wsmatch_type &m) {
			return m.prefix().str();
		}

		std::u32string suffix(const wsmatch_type &m) {
			return m.suffix().str();
		}
	}

	cs::var make_wstring()
	{
		return cs::var::make<std::u32string>();
	}

	CNI_REGISTER(wstring, cs::var::make_constant<cs::type_t>(make_wstring, cs::type_id(typeid(std::u32string))))

	var build_wregex(const std::u32string &str)
	{
		return var::make<wregex_type>(str);
	}

	CNI(build_wregex)
}

CNI_ENABLE_TYPE_EXT_V(codecvt,      codecvt_type,   "unicode::codecvt")
CNI_ENABLE_TYPE_EXT_V(wchar,        char32_t,       "unicode::wchar")
CNI_ENABLE_TYPE_EXT_V(wstring_type, std::u32string, "unicode::wstring")
CNI_ENABLE_TYPE_EXT_V(wregex,       wregex_type,    "unicode::wregex")
CNI_ENABLE_TYPE_EXT_V(wsmatch,      wsmatch_type,   "unicode::wregex::result")
