import math, re, time
import requests
import pandas as pd
from playwright.sync_api import sync_playwright

def scrape_ulta_fragrance(output_excel_path='ulta_womens_fragrance.xlsx'):
    # 1) Site constants
    BASE_URL     = 'https://www.ulta.com/fragrance?_request=true'
    API_ENDPOINT = 'https://www.ulta.com/api/product/search'
    PAGE_SIZE    = 60

    # 2) Spin up headless browser just to harvest cookies & X-UA-ID
    with sync_playwright() as pw:
        browser = pw.chromium.launch(headless=True)
        ctx     = browser.new_context(
            user_agent=(
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) "
                "AppleWebKit/537.36 (KHTML, like Gecko) "
                "Chrome/115.0.0.0 Safari/537.36"
            )
        )
        page = ctx.new_page()
        # navigate to women's fragrance category
        page.goto("https://www.ulta.com/fragrance?productType=women", wait_until="networkidle")
        cookies = ctx.cookies()
        # ULTA sometimes sets an X-UA-ID header via JS – grab it if present
        try:
            xua = page.evaluate("window.localStorage.getItem('ULTAPRODUCTSTORAGE_XUA');")
        except:
            xua = None
        browser.close()

    # 3) Seed into requests.Session()
    sess = requests.Session()
    for c in cookies:
        sess.cookies.set(c['name'], c['value'], domain=c['domain'], path=c['path'])
    headers = {
        'User-Agent': page.evaluate("navigator.userAgent"),
        'Accept': 'application/json, text/plain, */*',
        'Referer': 'https://www.ulta.com/fragrance?productType=women',
    }
    if xua:
        headers['X-UA-ID'] = xua
    sess.headers.update(headers)

    # 4) First call to get total count
    params = {
        'keyword': '',
        'categoryId': 'fragrance-women',  # ULTA’s URL uses this slug
        'siteId': 'ul', 
        'currentPage': 1,
        'pageSize': PAGE_SIZE
    }
    resp = sess.get(API_ENDPOINT, params=params, timeout=10)
    resp.raise_for_status()
    js = resp.json()
    total = js['recordsFiltered']
    pages = math.ceil(total / PAGE_SIZE)
    print(f"Found {total} products over {pages} pages on ULTA.")

    # 5) Loop and collect
    all_items = []
    for pg in range(1, pages+1):
        params['currentPage'] = pg
        resp = sess.get(API_ENDPOINT, params=params, timeout=10); resp.raise_for_status()
        data = resp.json()['products']
        print(f"Page {pg}/{pages} → {len(data)} items")

        for prod in data:
            brand = prod.get('brandName')
            name  = prod.get('productDisplayName')

            # price: ULTA returns cents
            price_cents = prod.get('standardRetailPrice', {}).get('amount')
            price = f"${price_cents/100:.2f}" if price_cents else None

            # size lives in a free-text attribute
            size_ml = None
            for attr in prod.get('attributes', []):
                val = attr.get('displayValue','')
                if 'mL' in val:
                    m = re.search(r'([\d\.]+)', val); size_ml = float(m.group(1)) if m else None
                    break
                if 'oz' in val:
                    m = re.search(r'([\d\.]+)', val)
                    size_ml = round(float(m.group(1)) * 29.5735,1) if m else None
                    break

            all_items.append({
                'Brand': brand,
                'Product Name': name,
                'Price': price,
                'Size (mL)': size_ml
            })

        time.sleep(0.1)

    # 6) Export to Excel
    df = pd.DataFrame(all_items)
    df.to_excel(output_excel_path, index=False)
    print(f" Saved {len(df)} SKUs to '{output_excel_path}'")

if __name__ == '__main__':
    scrape_ulta_fragrance()
