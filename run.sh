#!/bin/bash

export AUTH_TOKEN=$(cat auth_token.txt)

kraft run -e AUTH_TOKEN=${AUTH_TOKEN} --rm -p 8080:8080 --plat qemu --arch x86_64 .