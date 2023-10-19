@echo off

@chcp 65001

rem import open62541 in deps folder
git clone -b v1.3.8 --recursive git@github.com:open62541/open62541.git deps/open62541