#!/bin/bash

echo "Building GP2040-CE"

start_time=`date +%s`

# prep files
mkdir -p artifacts
chmod +x /usr/src/app/tools/makefsdata

# build web configurator
echo "Building Web Configurator with Node $(node -v)"
cd www
# npm install # only need to run this once -- can comment out after first run
npm run build

# build firmware
echo "Building Firmware"
cd -
PICO_SDK_FETCH_FROM_GIT=true cmake -B build/ -DCMAKE_BUILD_TYPE=Release
cmake --build build/ --parallel

# copy to artifacts folder and append timestamp to filename for lazy versioning
cp build/GP2040-CE_0.6.0_Pico.uf2 artifacts/GP2040-CE_0.6.0_Pico_$(date +%s).uf2

end_time=`date +%s`

exeuction_seconds=`expr $end_time - $start_time`

echo "execution time was $exeuction_seconds seconds."

# notify me when build is done (you'll probably want to comment this out)
curl -X POST -H "Content-Type: application/json" \
    -d "{\"app\": \"${NOTIFY_APP}\", \"title\": \"GP2040-CE\", \"message\": \"Finished compiling\n$exeuction_seconds seconds!\", \"sound\": \"custom1\", \"auth\": \"${NOTIFY_KEY}\"}" \
    $NOTIFY_URL