FROM ultrainio/builder as builder

RUN git clone -b master --depth 1 https://github.com/ULTRAINIO/ultrain.git --recursive \
    && cd ultrain \
    && cmake -H. -B"/tmp/build" -GNinja -DCMAKE_BUILD_TYPE=Release -DWASM_ROOT=/opt/wasm -DCMAKE_CXX_COMPILER=clang++ \
       -DCMAKE_C_COMPILER=clang -DCMAKE_INSTALL_PREFIX=/tmp/build  -DSecp256k1_ROOT_DIR=/usr/local \
    && cmake --build /tmp/build --target install


FROM ubuntu:16.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get -y install openssl && rm -rf /var/lib/apt/lists/*
COPY --from=builder /usr/local/lib/* /usr/local/lib/
COPY --from=builder /tmp/build/bin /opt/ultrainio/bin
COPY --from=builder /tmp/build/contracts /contracts
COPY --from=builder /ultrain/Docker/config.ini /ultrain/genesis.json /
COPY nodultraind.sh /opt/ultrainio/bin/nodultraind.sh
ENV ULTRAINIO_ROOT=/opt/ultrainio
RUN chmod +x /opt/ultrainio/bin/nodultraind.sh
ENV LD_LIBRARY_PATH /usr/local/lib
VOLUME /opt/ultrainio/bin/data-dir
ENV PATH /opt/ultrainio/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
