FROM gcc:13.3.0-bookworm AS build

WORKDIR /app

COPY src ./src
COPY Makefile .

RUN ls && make

FROM scratch

COPY --from=build /app/http_server /http_server
COPY --from=build /lib/x86_64-linux-gnu/libc.so.6 /lib/x86_64-linux-gnu/
COPY --from=build /lib64/ld-linux-x86-64.so.2 /lib64/