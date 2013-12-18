<?php

function cp_parser_page() {
    $catalogs = array();
    
    $lamoda_cat_count = db_query("SELECT COUNT(id) FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => 'lamoda.ru'
    ))->fetchField();
    $catalogs[] = array(
        array('data' => 'lamoda.ru'),
        array('data' => ''),//number_format($lamoda_cat_count, 0, '.', ' ')),
        array('data' => l('Управление отображением', 'parser/visibility/lamoda.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/lamoda.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/lamoda.ru/items'))
    );
    $catalogs[] = array(
        array('data' => 'ozon.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/ozon.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/ozon.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/ozon.ru/items'))
    );
    $catalogs[] = array(
        array('data' => 'neopod.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/neopod.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/neopod.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/neopod.ru/items'))
    );
    $catalogs[] = array(
        array('data' => '4glaza.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/4glaza.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/4glaza.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/4glaza.ru/items'))
    );
    $catalogs[] = array(
        array('data' => 'rc-today.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/rc-today.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/rc-today.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/rc-today.ru/items'))
    );
    $catalogs[] = array(
        array('data' => 'sledopyt.su'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/sledopyt.su').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/sledopyt.su').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/sledopyt.su/items'))
    );
    $catalogs[] = array(
        array('data' => 'bestbijoux.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/bestbijoux.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/bestbijoux.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/bestbijoux.ru/items'))
    );
    $catalogs[] = array(
        array('data' => 'cosmotop.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/cosmotop.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/cosmotop.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/cosmotop.ru/items'))
    );
    $catalogs[] = array(
        array('data' => 'evoi.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/evoi.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/evoi.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/evoi.ru/items'))
    );
    $catalogs[] = array(
        array('data' => 'confael.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/confael.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/confael.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/confael.ru/items'))
    );

	$catalogs[] = array(
        array('data' => 'first-store.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/first-store.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/first-store.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/first-store.ru/items'))
    );

    $catalogs[] = array(
        array('data' => 'pleer.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/pleer.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/pleer.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/pleer.ru/items'))
    );
	
	$catalogs[] = array(
        array('data' => 'vesta-alpha.com'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/vesta-alpha.com').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/vesta-alpha.com').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/vesta-alpha.com/items'))
    );
	
	$catalogs[] = array(
        array('data' => 'techport.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/techport.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/techport.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/techport.ru/items'))
    );

	$catalogs[] = array(
        array('data' => 'muztorg.ru'),
        array('data' => ''),
        array('data' => l('Управление отображением', 'parser/visibility/muztorg.ru').'&nbsp;&nbsp;'.l('Парсинг категорий', 'parser/muztorg.ru').'&nbsp;&nbsp;'.l('Парсинг товаров', 'parser/muztorg.ru/items'))
    );
	
    $header = array(
        array('data' => 'Магазин'),
        array('data' => 'Количество категорий (включая все дочерние)'),
        array('data' => 'Действия')
    );
	
    
    return '<div class="parser-table">'.theme('table', array(
        'header' => $header,
        'rows'   => $catalogs
    )).'</div>';
}

function cp_parser_proccess_page($store) {
    
//    if (arg(2) == 'items') {
//        $result = _cp_parse_store_catalog_items($store);
//    } else {
        $result = _cp_parse_store_catalog($store);
//    }
    
    return $result;
}

function cp_parser_visibility_page($store) {
    
    drupal_add_js(drupal_get_path('module', 'cp').'/misc/cp.js');
    drupal_add_css(drupal_get_path('module', 'cp').'/misc/cp.css');
    
    $main_category = db_query("SELECT * FROM {wish2wish_categories} WHERE provider = :provider AND category_parent_id = '0'", array(
        ':provider' => $store
    ))->fetchObject();
	    
    if (!$main_category) {
        return 'Нет категорий';
    }
    
    $output = '<table class="category-table">';
    $output .= '<tbody>';
    $output .= '<tr class="lvl lvl-1"><td>Каталог</td></tr>';
    $output .= _cp_category_get_childs($main_category, 1);
    
//    $categories = db_query("SELECT * FROM {wish2wish_categories} WHERE provider = :provider", array(
//        ':provider' => $store
//    ));
    
    $output .= '</tbody>';
    $output .= '</table>';
    
//    $output = 
    
    return $output;
}