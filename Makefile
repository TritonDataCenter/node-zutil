#
# Copyright 2019 Joyent, Inc.
#

#
# node-zutil Makefile
#

TAP_EXEC = ./node_modules/.bin/tap
TEST_JOBS ?= 10
TEST_TIMEOUT_S ?= 1200
TEST_GLOB ?= *

# If running as root, 'npm install' will not run scripts, even the default
# 'install' script that runs node-gyp to build with binding.gyp.
# '--unsafe-perm' works around that.
ifeq ($(shell whoami),root)
	NPM_OPTS=--unsafe-perm
else
	NPM_OPTS=
endif

#
# Targets
#

.PHONY: all
all $(TAP_EXEC):
	npm install $(NPM_OPTS)

.PHONY: clean
clean:
	rm -rf build node_modules npm-debug.log zonecfg-*.tgz test.tap

.PHONY: test
test: | $(TAP_EXEC)
	$(TAP_EXEC) --timeout $(TEST_TIMEOUT_S) -j $(TEST_JOBS) \
		-o ./test.tap test/$(TEST_GLOB).test.js

.PHONY: play
play:
	echo ""
	-node -e "console.log(require('./').getzonestate('$(shell zonename)'))"

check:: check-version

# Ensure CHANGES.md and package.json have the same version.
.PHONY: check-version
check-version:
	@echo version is: $(shell cat package.json | json version)
	[[ `cat package.json | json version` == `grep '^## ' CHANGES.md | head -2 | tail -1 | awk '{print $$2}'` ]]

.PHONY: cutarelease
cutarelease: check-version
	[[ -z `git status --short` ]]  # If this fails, the working dir is dirty.
	@which json 2>/dev/null 1>/dev/null && \
		ver=$(shell json -f package.json version) && \
		name=$(shell json -f package.json name) && \
		publishedVer=$(shell npm view -j $(shell json -f package.json name)@$(shell json -f package.json version) version 2>/dev/null) && \
		if [[ -n "$$publishedVer" ]]; then \
		echo "error: $$name@$$ver is already published to npm"; \
		exit 1; \
		fi && \
		echo "** Are you sure you want to tag and publish $$name@$$ver to npm?" && \
		echo "** Enter to continue, Ctrl+C to abort." && \
		read
	ver=$(shell cat package.json | json version) && \
		date=$(shell date -u "+%Y-%m-%d") && \
		git tag -a "v$$ver" -m "version $$ver ($$date)" && \
		git push origin "v$$ver" && \
		npm publish
