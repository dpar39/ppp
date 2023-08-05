
import os
import json
from collections import OrderedDict


this_dir = os.path.dirname(__file__)

ps_file = os.path.join(this_dir, 'photo-standards.json')

with open(ps_file, encoding='utf-8') as fp:
    data = json.load(fp, object_hook=OrderedDict)

asset_icons = os.path.join(this_dir, '../../assets/passports')

for st in data:

    cc = st.get('id')[:2]
    is_passport = st.get('docType') == 'Passport'
    thumb = os.path.join(asset_icons, f'{cc}.png')
    if os.path.isfile(thumb) and is_passport:
        thumbnail = f'/assets/passports/{cc}.png'
    else:
        thumbnail = f'/assets/flags/{cc}.svg'

    st['thumbnail'] = thumbnail

with open(ps_file, 'w', encoding='utf-8') as fp:
    json.dump(data, fp, indent=4)
