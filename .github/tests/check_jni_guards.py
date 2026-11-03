"""Check that Lunar cache lookups fail closed; this does not exercise the JVM."""
from pathlib import Path
import re

source = Path('fusion-plus/src/base/sdk/strayCache.h').read_text(encoding='utf-8')
start = source.index('if (Java::Version == MinecraftVersion::LUNAR_1_8_9)', source.index('static void Initialize()'))
end = source.index('else if (Java::Version == MinecraftVersion::LUNAR_1_7_10)', start)
lines = source[start:end].splitlines()
classes = lookups = 0
for index, line in enumerate(lines):
    if 'Java::AssignClass(' in line:
        classes += 1
        assert re.search(r'if\s*\(!Java::AssignClass\(.*\)\).*failureName\s*=.*return;', line), (index, line)
    if re.search(r'Java::Env->Get(?:Static)?(?:Field|Method)ID\(', line):
        lookups += 1
        name = re.match(r'\s*(\w+)\s*=', line)
        assert name, (index, line)
        guard = next((item.strip() for item in lines[index + 1:] if item.strip()), '')
        assert re.match(r'if \(!' + name.group(1) + r' \|\| Java::Env->ExceptionCheck\(\)\).*return;', guard), (index, guard)
assert classes and lookups, 'No Lunar 1.8.9 lookups found'
print(f'Checked {classes} class lookups and {lookups} field/method guards.')

