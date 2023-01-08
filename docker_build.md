# GP2040-CE Docker Build

If you want to extract docker build functionality from this repo, you'll need

 - [build-tools/build.sh](build-tools/build.sh)
    - I have it notify me when it's done, but you'll probably want to remove this from the end of the file as it won't work for you. 
 - [docker-compose.yml](docker-compose.yml)
    - You probably want to remove the `env_file` line -- this is only being used by the notification system mentioned above
 - [Dockerfile](Dockerfile)