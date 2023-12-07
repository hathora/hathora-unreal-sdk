## Build image

FROM gcr.io/distroless/cc-debian11:debug-nonroot

SHELL ["/busybox/sh", "-c"]

ARG game_server_exe=/home/nonroot/server/SDKDemo/Binaries/Linux/SDKDemoServer

# Copy the Unreal game server contents
COPY --chown=nonroot:nonroot ./builds/LinuxServer /home/nonroot/server

RUN chmod +x /home/nonroot/server/SDKDemo/Binaries/Linux/SDKDemoServer

## Final image

FROM gcr.io/distroless/cc-debian11:nonroot

COPY --chown=nonroot:nonroot --from=0 /home/nonroot/server /home/nonroot/server

# Expose the Unreal game server port
EXPOSE 7777/udp

ENTRYPOINT ["/home/nonroot/server/SDKDemo/Binaries/Linux/SDKDemoServer"]
