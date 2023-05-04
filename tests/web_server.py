import asyncio

import aiohttp
from aiohttp import web

default_headers = {"Date": "", "Server": ""}


async def test_1(request):
    text = "Hello world"
    return web.Response(text=text, headers=default_headers)


async def test_2(request):
    text = "Hello world"
    return web.Response(text=text, headers=default_headers)


TESTS = {
    8001: test_1,
    8002: test_2,
}

loop = asyncio.new_event_loop()
for port in TESTS:
    app = web.Application()
    app.add_routes([web.get('/', TESTS[port]), ])

    runner = aiohttp.web.AppRunner(app)
    loop.run_until_complete(runner.setup())
    site = aiohttp.web.TCPSite(runner, port=port)
    loop.run_until_complete(site.start())

loop.run_forever()
