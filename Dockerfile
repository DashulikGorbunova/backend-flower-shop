FROM gcc:latest as builder
WORKDIR /app
COPY . .
RUN g++ -std=c++17 -o server src/server.cpp

FROM ubuntu:22.04
WORKDIR /app
COPY --from=builder /app/server /app/server
COPY static/ /app/static/
COPY templates/ /app/templates/
EXPOSE 8080
CMD ["./server"]