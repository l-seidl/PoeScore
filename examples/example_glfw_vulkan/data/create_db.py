import os
import json
import sqlite3

DB_NAME = 'poe_data.db'

print("Start creating DB")

class Database:
    def __init__(self):
        self.connection = sqlite3.connect(DB_NAME)
        self.cursor = self.connection.cursor()

    def __del__(self):
        self.connection.close()

    def insert_mods(self):
        pass

    
    def create_db(self):
        self.cursor.execute('''DROP table IF EXISTS mods''')

        self.cursor.execute('''CREATE TABLE mods
            (id integer primary key, date integer, symbol text, low real, high real, open real, close real, volume real)''')


def write_mods_file():
    affixes = []

    def filter_mods():
        script_dir = os.path.dirname(__file__)
        abs_file_path = os.path.join( script_dir, "RePoE/RePoE/data/mods.min.json" )
        f = open(abs_file_path)

        mods = json.load(f)

        for mod_key in mods:
            mod = mods[mod_key]
            if mod['domain'] != "item": # for now only items, thinks to add: jewels, crafted, flasks ...
                continue

            if mod['generation_type'] != "prefix" and mod['generation_type'] != "suffix":
                continue

            new_affix = {
                "domain": mod["domain"],
                "generation_type": mod["generation_type"],
                "is_essence_only": mod["is_essence_only"],
                "implicit_tags": mod["implicit_tags"],
                "name": mod["name"],
                "required_level": mod["required_level"],
                "spawn_weights": mod["spawn_weights"],
                "stats": mod["stats"],
                "types": mod["type"],
            }

            affixes.append( new_affix )

    filter_mods()

    print( print("{}", len(affixes) ) )

    with open("mods_filtered.json", "w") as outfile:
        outfile.write( json.dumps(affixes) )


def write_base_items_file():
    base_items = []

    def filter_base_items():
        script_dir = os.path.dirname(__file__)
        abs_file_path = os.path.join( script_dir, "RePoE/RePoE/data/base_items.min.json" )
        f = open(abs_file_path)

        items = json.load(f)

        for item_key in items:
            item = items[item_key]
            if item['domain'] != "item": # for now only items, thinks to add: jewels, crafted, flasks ...
                continue

            new_item = {
                "domain": item["domain"],
                "item_class": item["item_class"],
                "name": item["name"],
                "tags": item["tags"],
            }

            base_items.append( new_item )

    filter_base_items()

    print( print("{}", len(base_items) ) )

    with open("base_items_filtered.json", "w") as outfile:
        outfile.write( json.dumps(base_items) )

def write_mode_translations_file():
    translations = []

    def filter_translations():
        script_dir = os.path.dirname(__file__)
        abs_file_path = os.path.join( script_dir, "RePoE/RePoE/data/stat_translations/advanced_mod.min.json" )
        f = open(abs_file_path)

        jsonTranslations = json.load(f)

        for translation in jsonTranslations:
            english = translation['English']

            if len(translation['ids']) != 1:
                print(json.dumps(translation['ids'], indent=4))

            new_translation = {
                "translations": [],
                "ids": translation["ids"],
            }

            for condition in english:
                regexSring = condition["string"]
                regexSring = regexSring.replace("(", "\\(")
                regexSring = regexSring.replace(")", "\\)")

                counter = 0
                for format in condition["format"]:
                    if format == '#':
                        findStr = "{" + str(counter) + "}"
                        regexSring = regexSring.replace( findStr, '([\\.\\d]+)[-()\\.\\d]*' )
                    elif format == '+#':
                        findStr = "{" + str(counter) + "}"
                        regexSring = regexSring.replace( findStr, '([+-][\\.\\d]+)[-()\\.\\d]*' )
                    counter = counter + 1

                regexSring = regexSring.replace("\n", "\r")
                # regexSring = regexSring.replace("%", "\\%")
                new_translation["translations"].append({
                    "string": condition["string"],
                    "format": condition["format"],
                    "regex": regexSring,
                })
#                new_translation["regexStrings"].append(cond['string'])
#                new_translation["cond"].append(cond['condition'])
                


            translations.append( new_translation )

    filter_translations()

    print( print("{}", len(translations) ) )

    with open("mod_translations.json", "w") as outfile:
        outfile.write( json.dumps(translations, indent=4) )

write_mode_translations_file()
# write_base_items_file()
# write_mods_file()
