---
layout: page
title: API Documentation
permalink: /apidoc/
nav_order: 2
---
{% assign urlapidoc = site.url | append: site.baseurl | append: "/artifacts/apidoc/dragonscript/" %}
{% assign versions = site.data.apidoc.dragonscript %}

# API Documentation

API documentation of the DragonScript Language (*.ds).

## Latest
{% assign version = versions.first.version %}
[DragonScript Language: {{ version }}]({{ urlapidoc }}{{ version }}/index.html){:target="_blank"}

## Previous Versions
{% assign count = versions.size | minus: 1 %}
{% assign versions2 = versions | slice: 1, count %}
{% for each in versions2 %}
  {% assign version = each.version %}
  [DragonScript Language: {{ version }}]({{ urlapidoc }}{{ version }}/index.html){:target="_blank"}
{% endfor %}
