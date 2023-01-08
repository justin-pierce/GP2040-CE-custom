#!/bin/bash

echo "Building GP2040-CE"

# prep files
mkdir artifacts
chmod +x /usr/src/app/tools/makefsdata

# build web configurator
cd www
# npm install # only need to run this once -- can comment out after first run
npm run build

# build firmware
cd -
PICO_SDK_FETCH_FROM_GIT=true cmake -B build/ -DCMAKE_BUILD_TYPE=Release
cmake --build build/

# copy to artifacts folder and append timestamp to filename for lazy versioning
timestamp=$(date +%s)
cp build/GP2040-CE_0.6.0_Pico.uf2 artifacts/GP2040-CE_0.6.0_Pico_$timestamp.uf2

# notify me when build is done (you'll probably want to comment this out)
curl -X POST -H "Content-Type: application/json" \
    -d "{\"app\": \"${NOTIFY_APP}\", \"title\": \"GP2040-CE\", \"message\": \"Finished compiling!\", \"sound\": \"custom1\", \"auth\": \"${NOTIFY_KEY}\"}" \
    $NOTIFY_URL