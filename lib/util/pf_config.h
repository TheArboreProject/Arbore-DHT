/* LibConfig.h - This is a library to make a configuration
 *
 * Copyright (C) 2006 Romain Bignon  <progs@headfucking.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

/* !WARNING! This library must be used in a program with this function :
 *           * std::string stringtok(std::string &in, const char * const delimiters);
 */

#ifndef PF_CONFIG_H
#define PF_CONFIG_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <ctype.h>
#include <limits.h>

class ConfigSection;
class ConfigItem;

typedef std::multimap<std::string, ConfigSection*> SectionMap;
typedef std::map<std::string, ConfigItem*> ItemMap;

/********************************************************************************************
 *                                Config                                                    *
 ********************************************************************************************/
/** @page MyConfig Use MyConfig classes
 *
 * This library can be used to read a configuration in this form :
 *
 * @code
 * section {
 *      label = value
 *      subsection {
 *           subsection2 {
 *                 label = value
 *                 label2 = value2
 *           }
 *           label = value
 *      }
 * }
 *
 * section2 {
 *       ...
 * }
 * @endcode
 *
 * You have to create an instance of class \b MyConfig .
 * Next, define all sections, subsections and items. If there is an error, a MyConfig::error exception is sended.
 * Then, try to load configuration.
 *
 * \b ConfigSection is class for a section in configuration. Add one section in MyConfig or in an other
 * ConfigSection (his parent).
 *
 * There is a few types of item classes. Each class is derived from \b ConfigItem and have some features
 * about his type, condition to a valid value, etc.
 *
 * \b ConfigItem has some functions to return a valid in a few types. This functions are \b String(), \b Integer()
 * and \b Boolean().
 *
 * You can create your own derived class, read \b ConfigItem interface and a derived class from \b ConfigItem to
 * know what you have to do. Existing derived classed are \b ConfigItem_string, \b ConfigItem_int and
 * \b ConfigItem_bool.
 *
 * There is an example :
 *
 * @code
 * // Creation of a new instance of MyConfig. It will read "app.cfg".
 * MyConfig* config = new MyConfig("app.cfg");
 *
 * // Use of a try/catch bloc, to print an error if we catch an exception
 * try
 * {
 *    // This first section is a no-forkable section
 *    ConfigSection* section = config->AddSection("section", "this is my first section", false);
 *    // We add a new item. His value must be a string
 *    section->AddItem(new ConfigItem_string("str", "a string item"));
 *    // An integer item
 *    section->AddItem(new ConfigItem_int("int", "an integer item"));
 *    // We add a forkable subsection.
 *    section = section->AddSection("subsection", "a subsection of my first section", true);
 *    // This item has "is_name" true. So his value will be the identificator of this fork
 *    section->AddItem(new ConfigItem_string("name", "name of the subsection"), true);
 *    section->AddItem(new ConfigItem_string("description", "description of this subsection"));
 *    // We return to parent section ("section")
 *    section = section->Parent();
 *    // A new subsection, no-forkable
 *    section = section->AddSection("subsection2", "a second subsection", false);
 *    section->AddItem(new ConfigItem_int("something", "i don't know"));
 *    section = config->AddSection("section2", "my second head-section", false);
 *    // Here we suppose there is a ConfigItem_ip type. We can imagine that ConfigItem_ip::SetValue() will
 *    // check if value is a real ip.
 *    section->AddItem(new ConfigItem_ip("ip", "ip of a server"));
 *
 *    // Now we can load configuration, and pray there aren't any error.
 *    // This function will not send any exception, but if there is any error in configuration, it will
 *    // print messages (each times) and return false.
 *    return config->Load();
 * }
 * catch(MyConfig::error &e)
 * {
 *    std::cerr << "I catch an error from MyConfig : " << e.Reason() << std::endl;
 *    delete config;
 *    return false;
 * }
 * @endcode
 *
 */
class MyConfig
{
	/* Constructeur */
public:
	MyConfig(std::string path);

	MyConfig();

	~MyConfig();

	/* Exceptions */
public:

	/** This class is an exception class throwed when there is a problem in building of sections/items */
	class error_exc
	{
		public:
			error_exc(std::string reason) : reason_(reason) {}
			std::string Reason() const { return reason_; }
		private:
			std::string reason_;
	};

	/* Methodes */
public:

	bool Load(std::string path = "");

	bool FindEmpty();

	/* Attributs */
public:

	/** Get a section from his label */
	ConfigSection* GetSection(std::string label);

	/** Get a section fork with his label and his name.
	 * This function works only with a section marcked "multiple". It will return the fork of this section.
	 * @param label this is the label of the section
	 * @param name name of the section
	 */
	ConfigSection* GetSection(std::string label, std::string name);

	/** Get clones of a section marcked "multiple" */
	std::vector<ConfigSection*> GetSectionClones(std::string label);

	/** Add a head-section in the configuration.
	 * Before loading configuration, you have to define all sections and items. This function must be
	 * used to add a head-section.
	 * @param label section's label, this is on the identificator in the configuration ("label {")
	 * @param description section's description, sended to user when this section is forgotten
	 * @param multiple if setted, this section can have multiple definitions (see ConfigSection::AddItem)
	 */
	ConfigSection* AddSection(std::string label, std::string description, bool multiple);

	unsigned NbLines() const { return line_count_; }
	std::string Path() const { return path_; }

private:
	ConfigSection* AddSection(ConfigSection*);

	/* Variables privées */
private:
	std::string path_;
	bool loaded_;
	SectionMap sections_;
	unsigned int line_count_;
};

/********************************************************************************************
 *                                ConfigSection                                             *
 ********************************************************************************************/

class ConfigSection
{
	/* Constructeur */
public:
	ConfigSection(ConfigSection&);

	~ConfigSection();

	typedef void (*TEndOfTab) (ConfigSection*);

	friend class MyConfig;
	/* Methodes */
public:

	/** Get a subsection from his label */
	ConfigSection* GetSection(std::string label);

	/** Get a subsection fork with his label and his name.
	 * This function works only with a section marcked "multiple". It will return the fork of this section
	 * whose have the searched name.
	 * @param label this is the label of the subsection
	 * @param name name of the subsection
	 */
	ConfigSection* GetSection(std::string label, std::string name);

	/** Get clones of a section marcked "multiple" */
	std::vector<ConfigSection*> GetSectionClones(std::string label);

	/** Add a subsection in the configuration.
	 * Before loading configuration, you have to define all sections and items. This function must be
	 * used to add a subsection.
	 * @param label section's label, this is on the identificator in the configuration ("label {")
	 * @param description section's description, sended to user when this section is forgotten
	 * @param multiple if setted, this section can have multiple definitions (see ConfigSection::AddItem)
	 */
	ConfigSection* AddSection(std::string label, std::string description, bool multiple);

	/** Get an item from his label */
	ConfigItem* GetItem(std::string label);

	/** This function add an item in this section.
	 * If this item is a member of a multiple section (see ConfigSection::AddSection), an item have to be an
	 * identificator of this section. Set "is_name" if this item is one.
	 * @param item this is a new item of a derived type from ConfigItem. It will be deleted if there is an exception
	 * @param is_name if true, this is the name (identificator) of the section.
	 */
	void AddItem(ConfigItem* item, bool is_name = false);

	/** Is this is a multiple struct, name is an identificator of this type of struct in the configuration */
	std::string Name() const { return name_; }
	void SetName(std::string name) { name_ = name; }

	/** Label is an identificator in the configuration « label { }; » */
	std::string Label() const { return label_; }

	std::string Description() const { return description_; }

	/** If this is true, this section can be forked */
	bool IsMultiple() const { return multiple_; }

	ConfigSection* Parent() const { return parent_; }

	bool IsCopy() const { return copy_; }
	void SetCopy() { copy_ = true; }

	/** NameItem is name's item of this section */
	ConfigItem* NameItem() const { return name_item_; }

	/** If true, this item is found in the configuration */
	bool Found() const { return found_; }
	void SetFound() { found_ = true; }

	/** This function will be called when parser is at the end of the section.
	 * @param eot_f a function in form « void Name (ConfigSection*); »
	 */
	void SetEndOfTab(TEndOfTab eot_f) { end_func_ = eot_f; }
	TEndOfTab EndOfTab() const { return end_func_; }

private:
	/** This constructor is privated because only AddSection must call him. */
	ConfigSection(std::string name, std::string description, bool multiple, MyConfig* config, ConfigSection* parent);

	bool FindEmpty();
	ConfigSection* AddSection(ConfigSection*);

	/* Variables privées */
private:
	std::string name_;
	std::string label_;
	std::string description_;
	bool multiple_;
	ItemMap items_;
	SectionMap sections_;
	MyConfig* config_;
	ConfigSection* parent_;
	bool copy_;
	ConfigItem* name_item_;
	bool found_;
	TEndOfTab end_func_;
};

/********************************************************************************************
 *                                ConfigItem                                                *
 ********************************************************************************************/
/** This is an abstract class whose represent an Item in pair \a label=value .
 * You have to create a derived class from this class to have a new type.
 * Overload « Integer() » and « String() » to return value in each forms.
 * Overload « SetValue(std::string) » to check value and cast to type.
 */
class ConfigItem
{
	/* Constructeur */
public:

	typedef void (*TCallBack) (ConfigItem*);

	/** \b ConfigItem is a section's item.
	 * @param label this is the identificator of the item (in pair \a label=value )
	 * @param description description of this item
	 * @param def_value default value, if isn't setted, this item is needed
	 * @param call_back this is a pointer to a function called before setting this item
	 * @param config a pointer to the MyConfig instance
	 * @param parent parent of this item (a ConfigSection)
	 */
	ConfigItem(std::string label, std::string description, std::string def_value, TCallBack call_back,
		MyConfig* config, ConfigSection* parent)
		: label_(label), description_(description), config_(config), parent_(parent), found_(false),
		call_back_(call_back), def_value_(def_value)
		{}

	virtual ~ConfigItem() {}

	friend class ConfigSection;

	/* Methodes */
public:

	/** Give a clone of this Item (you couldn't be allowed to call this) */
	virtual ConfigItem* Clone() const = 0;

	/* Attributs */
public:

	std::string Label() const { return label_; }
	std::string Description() const { return description_; }

	ConfigSection* Parent() const { return parent_; }
	MyConfig* GetConfig() const { return config_; }

	/** If true, this item was found on the configuration by parser */
	bool Found() const { return found_; }
	void SetFound() { found_ = true; }

	virtual int Integer() const	  /**< Get value as an integer */
	{
		return 0;
	}
	virtual std::string String() const/**< Get value as a string */
	{
		return "";
	}
	virtual bool Boolean() const	  /**< Get a boolean value */
	{
		return false;
	}
	/**< This function might return false if value isn't good */
	virtual bool SetValue(std::string) = 0;

	virtual std::string ValueType() const = 0;

	TCallBack CallBack() const { return call_back_; }

	std::string DefValue() const { return def_value_; }

	/* Variables privées */
private:
	std::string label_;
	std::string description_;
	MyConfig* config_;
	ConfigSection* parent_;
	bool found_;
	TCallBack call_back_;
	std::string def_value_;
};

/** This is a derived class from ConfigItem whose represent a string item */
class ConfigItem_string : public ConfigItem
{
public:
	ConfigItem_string(std::string label, std::string description, std::string def_value = "", TCallBack cb = 0,
		MyConfig* config = 0, ConfigSection* parent = 0)
		: ConfigItem(label, description, def_value, cb, config, parent)
		{}

	virtual ConfigItem* Clone() const
	{
		return new ConfigItem_string(Label(), Description(), DefValue(), CallBack(), GetConfig(), Parent());
	}

	virtual std::string String() const { return value_; }
	virtual bool SetValue(std::string s) { value_ = s; return true; }

	std::string ValueType() const { return "string"; }

private:
	std::string value_;
};

/** This is a derived class from ConfigItem whose represent an integer item */
class ConfigItem_int : public ConfigItem
{
public:
	ConfigItem_int(std::string label, std::string description, int min = INT_MIN, int max = INT_MAX, std::string def_value = "",
		TCallBack cb = 0, MyConfig* config = 0, ConfigSection* parent = 0)
		: ConfigItem(label, description, def_value, cb, config, parent), min_(min), max_(max)
		{}

	virtual ConfigItem* Clone() const;

	/** We return a string form of this integer */
	virtual std::string String() const { std::ostringstream oss; oss << value_; return oss.str(); }

	virtual int Integer() const { return value_; }
	virtual bool SetValue(std::string s);
	std::string ValueType() const;

private:
	int value_, min_, max_;
};

/** This is a derived class from ConfigItem whose represent an integer item */
class ConfigItem_bool : public ConfigItem
{
public:
	ConfigItem_bool(std::string label, std::string description, std::string def_value = "", TCallBack cb = 0,
		MyConfig* config = 0, ConfigSection* parent = 0)
		: ConfigItem(label, description, def_value, cb, config, parent)
		{}

	virtual ConfigItem* Clone() const;
	/** We return a string form of this integer */
	virtual std::string String() const { return (value_ ? "true" : "false"); }

	virtual int Integer() const { return value_; }
	virtual bool Boolean() const { return value_; }
	virtual bool SetValue(std::string s);

	std::string ValueType() const;

private:
	bool value_;
};

extern MyConfig conf;
#endif						  /* LIBCONFIG_H */
