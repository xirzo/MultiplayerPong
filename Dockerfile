FROM alpine:3.22.2 AS build

WORKDIR /src

COPY ./bin/ ./bin/
COPY ./lib/ ./lib/
COPY ./configuration.game .
COPY ./CMakeLists.txt .

RUN apk add --no-cache \
    build-base \
    cmake \
    git \
    mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libxcursor-dev \
    libxinerama-dev \  
    libxext-dev \     
    alsa-lib-dev \
    libbsd-dev

RUN cmake -DFETCH_LIBS=ON -B build/
RUN cmake --build build/

FROM alpine:3.22.2

RUN apk add --no-cache \
    libstdc++ \
    mesa \
    libx11 \
    libxrandr \
    libxi \
    libxcursor \
    libxinerama \
    libxext \
    alsa-lib

WORKDIR /app

COPY --from=build /src/configuration.game /app/
COPY --from=build /src/build/bin/pong_server /app/

# starts on port 8080
CMD ["./pong_server"]
