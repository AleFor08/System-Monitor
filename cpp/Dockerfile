FROM yhirose4dockerhub/ubuntu-builder AS builder
WORKDIR /build
COPY include/cpp-httplib-0.30.1/httplib.h .
COPY include/cpp-httplib-0.30.1/docker/main.cc .
RUN g++ -std=c++23 -static -o server -O2 -I. main.cc && strip server

FROM scratch
COPY --from=builder /build/server /server
COPY include/cpp-httplib-0.30.1/docker/html/index.html /html/index.html
EXPOSE 80

ENTRYPOINT ["/server"]
CMD ["--host", "0.0.0.0", "--port", "80", "--mount", "/:./html"]
