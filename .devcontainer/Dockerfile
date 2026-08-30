FROM cpp-base

RUN apt-get update \
	&& apt-get install -y --no-install-recommends clang-tidy tree clang-format git cmake ca-certificates \
	&& apt-get install -y --no-install-recommends pkg-config libsdl2-dev libimgui-dev \
	&& rm -rf /var/lib/apt/lists/*

RUN git clone --branch 13.0.0 --depth 1 https://github.com/xtensor-stack/xsimd.git /tmp/xsimd \
	&& cmake -S /tmp/xsimd -B /tmp/xsimd/build -DBUILD_TESTS=OFF \
	&& cmake --install /tmp/xsimd/build \
	&& rm -rf /tmp/xsimd

RUN echo "alias build='cmake --build build -j$(nproc)'" >> ~/.bashrc \
    && echo "alias run='build && ./build/territory_capture_gui'" >> ~/.bashrc

ENV CC=clang
ENV CXX=clang++