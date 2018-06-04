FROM ultrainio/builder
ARG branch=master
ARG symbol=SYS

RUN git clone -b $branch https://github.com/ULTRAINIO/ultrain.git --recursive \
    && cd ultrain && echo "$branch:$(git rev-parse HEAD)" > /etc/ultrainio-version \
    && cmake -H. -B"/opt/ultrainio" -GNinja -DCMAKE_BUILD_TYPE=Release -DWASM_ROOT=/opt/wasm -DCMAKE_CXX_COMPILER=clang++ \
       -DCMAKE_C_COMPILER=clang -DCMAKE_INSTALL_PREFIX=/opt/ultrainio  -DSecp256k1_ROOT_DIR=/usr/local -DBUILD_MONGO_DB_PLUGIN=true -DCORE_SYMBOL_NAME=$symbol \
    && cmake --build /opt/ultrainio --target install \
    && mv /ultrain/Docker/config.ini / && mv /opt/ultrainio/contracts /contracts && mv /ultrain/Docker/nodultraind.sh /opt/ultrainio/bin/nodultraind.sh && mv tutorials /tutorials \
    && rm -rf /ultrain

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get -y install openssl vim psmisc python3-pip && rm -rf /var/lib/apt/lists/*
RUN pip3 install numpy
ENV ULTRAINIO_ROOT=/opt/ultrainio
RUN chmod +x /opt/ultrainio/bin/nodultraind.sh
ENV LD_LIBRARY_PATH /usr/local/lib
VOLUME /opt/ultrainio/bin/data-dir
ENV PATH /opt/ultrainio/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
