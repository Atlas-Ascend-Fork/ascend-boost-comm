import re
file_path = "src/include/mki/utils/rt/base/types.h"
with open(file_path, 'r') as f:
    content = f.read()

pattern = r"typedef struct \{(.*?)\} (\w+);"
fixed_content = re.sub(pattern, r"typedef struct \2 {\1} \2;", content, flags=re.DOTALL)

with open(file_path, 'w') as f:
    f.write(fixed_content)
