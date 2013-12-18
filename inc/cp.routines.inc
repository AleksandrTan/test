<?php

function _cp_load_data($prefix, $host, $file, $alternative_filename = null) {
    header('Content-Type: text/html; charset=utf-8');
    ini_set('display_errors', 1);
    error_reporting(~0);
	$start = microtime(true);

    if ($file == 'book') {
        return;
    }

    $errno = '';
    $errstr = '';
//    $host = 'static.ozone.ru';
//    $file = '/multimedia/yml/facet/div_kid.xml';
    $remote_file = fsockopen($host, 80, $errno, $errstr, 30) or die("облом");
	
	if (!file_exists("data/$prefix/"))
	{
		mkdir("data/$prefix/", 0777);
	}
	fputs($remote_file,"GET $file HTTP/1.0\nHOST: $host\n\n");
	//$remote_file = fopen("http://first-store.ru/partner_export.php", "r") or die('error');
    if ($remote_file) {
        $file_prefix = '';
        if ($prefix == 'ozon') {
            $file_prefix = 'div_';
        }
		if ($prefix == 'vestaalpha') {
            $file_prefix = 'div_';
        }
        if ($alternative_filename) {
            $local_file_name = "data/$prefix/$file_prefix$alternative_filename.xml";
        } else {
            $local_file_name = "data/$prefix/$file_prefix$prefix.xml";
        }
        if (file_exists($local_file_name)) {
            chmod($local_file_name, 0777);
        }
        $local_file = fopen($local_file_name, 'w+');
        if ($local_file) {
            $headerendfound = false;
            $buffer = '';
            while (!feof($remote_file)) {
                $out = fgets ($remote_file,4096);
                if ($headerendfound) {
                    fputs($local_file, $out);
                }
                if (!$headerendfound) {
                    $buffer .= $out;
                    $headerend = strpos($buffer, "\r\n\r\n");
                    if ($headerend !== false) {
                        $headerendfound = true;
                        fputs($local_file, substr($buffer, $headerend+4));
                        $buffer = '';
                    }
                }
            }
            fclose($local_file);
        } else {
            die('Не удалось открыть файл для записи');
        }
        fclose($remote_file);
    } else {
        die('Не удалось открыть файл http://affiliates.lamoda.ru/materials/downloadXml?name=affiliate');
    }

    print 'Загружено за '.number_format(microtime(true) - $start, 2, '.', ' ').' сек.<br />';
    $start = microtime(true);

    if($prefix == 'ozon') {
        print "Замена символов. ";
        $fromFile = $_SERVER['DOCUMENT_ROOT']."/".$local_file_name;
        $toFile = str_replace(".xml", "_fixed.xml", $fromFile);
        $comm = "cat ".$fromFile." | tr '\230' '\040' > ".$toFile;
        @unlink($toFile);
        print system($comm);
        print 'Выполнено за '.number_format(microtime(true) - $start, 2, '.', ' ').' сек.<br />';
    }

    return true;
}

function cp_category_save($cid) {
    $sex = (int)$_POST['sex'];
    $visibility = (int)$_POST['checked'];

    $updated = db_update('wish2wish_categories')
        ->fields(array('sex' => $sex, 'visibility' => $visibility))
        ->condition('id', $cid, '=')
        ->execute();
    print drupal_json_encode(array('result' => 1));
}

function _cp_category_get_childs($category, $lvl) {
    $categories = db_query("SELECT * FROM {wish2wish_categories} WHERE provider = :provider AND category_parent_id = :pid", array(
        ':provider' => $category->provider,
        ':pid' => $category->category_id
    ));
    $output = '';
    $main_categories = array('ozon', 'lamoda.ru');
    if ($categories->rowCount()) {
        $lvl++;
        foreach ($categories as $c) {
            $checked = $c->visibility == 1 ? ' checked="checked"' : '';
            $output .= '<tr class="lvl lvl-'.$lvl.' cat-'.$c->id.'"><td>'.$c->title.' <label>
                <input onclick="categoryChanged('.$c->id.')" type="checkbox"'.$checked.' name="enabled['.$c->id.']" /> Показывать</label>
                <select name="sex['.$c->id.']" onchange="categoryChanged('.$c->id.')">
                    <option value="0"'.($c->sex == 0 ? ' selected' :'').'>Не указано</option>
                    <option value="1"'.($c->sex == 1 ? ' selected' :'').'>Для женщин</option>
                    <option value="2"'.($c->sex == 2 ? ' selected' :'').'>Для мужчин</option>
                    <option value="3"'.($c->sex == 3 ? ' selected' :'').'>Для детей</option>
                </select>
                <input type="button" value="Сохранить" style="display: none;" onclick="categorySave('.$c->id.')" />
                </td></tr>';
            $output .= _cp_category_get_childs($c, $lvl);
        }
    }
    return $output;
}

function _cp_stores() {
    return array(
        'lamoda.ru' => array(
            'prefix' => 'lamoda'
        ),
        'ozon.ru' => array(
            'prefix' => 'ozon'
        ),
        'neopod.ru' => array(
            'prefix' => 'neopod'
        ),
        '4glaza.ru' => array(
            'prefix' => '4glaza'
        ),
        'rc-today.ru' => array(
            'prefix' => 'rctoday'
        ),
        'sledopyt.su' => array(
            'prefix' => 'sledopyt'
        ),
        'bestbijoux.ru' => array(
            'prefix' => 'bestbijoux'
        ),
        'cosmotop.ru' => array(
            'prefix' => 'cosmotop'
        ),
        'evoi.ru' => array(
            'prefix' => 'evoi'
        ),
        'confael.ru' => array(
            'prefix' => 'confael'
        ),
		'first-store.ru' => array(
			'prefix' => 'firststore'
		),
		'pleer.ru' => array(
			'prefix' => 'pleer'
		),
		'vesta-alpha.com' => array(
			'prefix' => 'vestaalpha'
		),
		'techport.ru' => array(
			'prefix' => 'techport'
		),
		
		'muztorg.ru' => array(
			'prefix' => 'muztorg'
		)

    );
}

function _cp_parse_store_catalog($store) {
    $stores = _cp_stores();
    
    if (!isset($stores[$store]['prefix'])) {
        drupal_not_found();
    }
    
    $prefix = $stores[$store]['prefix'];
    if (!function_exists("_cp_".$prefix."_parser")) {
        drupal_not_found();
    }
    
    $items = '';
    if (arg(2) == 'items') {
        $items = '_items';
        if (!function_exists("_cp_".$prefix.$items."_parser")) {
            drupal_not_found();
        }
    }
    
    $result = call_user_func("_cp_".$prefix.$items.'_parser', $prefix, $store);
    
    return $result;
}
/////////////////////////////////////DELETE PRODUCTS///////////////////////////////////////////////////////////////////
function _cp_delete_products($filename, $provider, $updated)
{
    if (!file_exists($filename)) {
        return;
    }

    $fileBasename = pathinfo($filename, PATHINFO_BASENAME);

    $or = db_or();
    $or->condition('filename', '');
    $or->condition('filename', $fileBasename);

    db_delete('wish2wish_products')
        ->condition($or)
        ->condition('provider', $provider)
        ->condition('updated', $updated, '<')
        ->execute();

}

function _cp_update_categories($filename, $provider, $prefix) {
    if (!file_exists($filename)) {
        return;
    }

    $updated = time();

    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        $cat = $prefix . '_' . (String)$node->attributes()->id;
        $catParent = isset($node->attributes()->parentId) ? $prefix . '_' . (String)$node->attributes()->parentId : '0';
        $catTitle = (String)$node[0];


        $category = db_query("SELECT * FROM {wish2wish_categories} WHERE category_id = :cat", array(':cat'=>$cat))->fetchObject();

        if ($category) {
            db_update('wish2wish_categories')
                ->fields(array(
                    'title' => $catTitle,
//                    'category_parent_id' => $catParent,
                    'updated' => $updated
                ))
                ->condition('id', $category->id)
                ->execute();
        } else {
            db_query("INSERT INTO {wish2wish_categories}
               (category_id, category_parent_id, sex, fid, title, provider, visibility, weight, updated)
                VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0, :updated)", array(
                ':category_id' => $cat,
                ':category_parent_id' => $catParent,
                ':title' => $catTitle,
                ':provider' => $provider,
                ':updated' => $updated
            ));
            $category = db_query("SELECT * FROM {wish2wish_categories} WHERE category_id = :cat", array(':cat'=>$cat))->fetchObject();
        }

        if ($category) {
            $keys = array(
                'category-data-'.$category->id,
                'category-subcats-data-'.$category->category_id,
                'category-has-items-'.$category->category_id,
                'category-'.$category->category_id.'-subcategories',
                'breadcrumbs-'.$category->id,
                'category-childrens-id-'.$category->id
            );

            db_delete('my_cache')
                ->condition('cache_key', $keys, 'IN')
                ->execute();

        }

        $reader->next('category');
    }

    return 'Done';
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

function _cp_lamoda_items_parser ($prefix, $provider, $step_param = null) {

    header('Content-Type: text/html; charset=utf-8');

    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'lamoda'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filters[$filter->fid] = $filter;
    }

    $filename = $_SERVER['DOCUMENT_ROOT'].'/data/lamoda/lamoda.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE 'lamoda_%'");
    }

    $fileBasename = 'lamoda.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = (String)$node->model;//'bestbijoux-'.$item_id;
            $search = (String)$node->typePrefix.' '.(String)$node->model;
            $type_vendor = (String)$node->typePrefix.' '.(String)$node->vendor;
            $params = array();
            $ratio = 0.10;
            $size_unit = '';
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;

                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                    if ($p_name == 'Размер') {
                        $size = (String)$value;
                        if ((String)$value->attributes()->unit) {
                            $size_unit = (String)$value->attributes()->unit;
                        }
                    }
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_id = (String)$node->attributes()->group_id;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['group_id'] = $group_id ? $group_id : 0;
                $item['picture'] = '';
                $item['type_prefix'] = (String)$node->typePrefix ? (String)$node->typePrefix : '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                $item['data']['size_unit'] = $size_unit;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            $value_exists = null;
                            if (isset($params[$f->name])) {
                                $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                    ':model' => $item->model,
                                    ':value' => $params[$f->name],
                                    ':cid' => $_category_id
                                ))->fetchField();
                            }

                            if (isset($params[$f->name]) && !$value_exists) {
                                db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                    ':fid' => $f->fid,
                                    ':model' => $item->model,
                                    ':value' => $params[$f->name],
                                    ':cid' => $_category_id,
                                    ':fmn' => $f->machine_name,
                                    ':f' => $fileBasename
                                ));
                            }
                            if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                    ':fid' => $f->fid,
                                    ':model' => $item->model,
                                    ':value' => $item->vendor,
                                    ':cid' => $_category_id,
                                    ':fmn' => $f->machine_name,
                                    ':f' => $fileBasename
                                ));
                            }
                        }
                    }
                    $counter++;

                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->type_prefix,
                            ':gid' => $item->group_id,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $type_vendor,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->type_prefix,
                            ':provider' => $provider,
                            ':gid' => $item->group_id,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $type_vendor,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}

function _cp_lamoda_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    $filename = $_SERVER['DOCUMENT_ROOT'].DIRECTORY_SEPARATOR.'data'.DIRECTORY_SEPARATOR.'affiliate.xml';
    if (!file_exists($filename)) {
        drupal_set_message('Файл '.$filename.' не найден', 'error');
        drupal_goto('parser');
    }

    $data = file_get_contents($filename);

    $output = '';
    $categories = array();
    if (preg_match_all('/<category[^>]*>.*<\/category>/', $data, $matches)) {
        foreach ($matches[0] as $category) {
            $current_category = array();
            $name = preg_match('/<category[^>]*>(.*)<\/category>/', $category, $category_name_match);
            $id = preg_match('/id=\'(\d+)\'/', $category, $category_id_match);
            $parentId = preg_match('/parentId=\'(\d+)\'/', $category, $category_parent_id_match);
            if ($name && $id) {
                $current_category['name'] = $category_name_match[1] == 'Default Category' ? 'lamoda.ru' : $category_name_match[1];
                $current_category['id'] = $category_id_match[1];
                $current_category['parent_id'] = $parentId ? $category_parent_id_match[1] : 0;
                $categories[] = $current_category;
            }
        }
    }

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => 'lamoda.ru'
    ));
    if (!empty($categories)) {
        foreach ($categories as $item) {
            $item = (object) $item;
            db_query("INSERT INTO {wish2wish_categories} VALUES (NULL, :id, :pid, 0, 0, :title, :provider)", array(
                ':id' => $prefix.'_'.$item->id,
                ':pid' => $item->parent_id ? $prefix.'_'.$item->parent_id : $item->parent_id,
                ':title' => $item->name,
                ':provider' => 'lamoda.ru'
            ));
        }
    }

    drupal_set_message('Категории lamoda.ru перестроены.');
    drupal_goto('parser');

    $output .= '<pre>'.print_r($categories, 1).'</pre>';

    unset($xml);
    unset($data);

    return $output;
}


function _cp_ozon_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    $catalogs_to_parse = array(
//        'book' => 'Книги',
//        'appliance' => 'Бытовая техника',
//        'beauty' => 'Красота и здоровье',
//        'bs' => 'Спорт и отдых',
//        'dvd' => 'DVD и Blu-ray',
//        'fashion' => 'Одежда, обувь, аксессуары',
//        'gifts' => 'Подарки и сувениры',
//        'home' => 'Дом, сад, зоотовары',
        'kid' => 'Детям и мамам',
//        'music' => 'Музыка',
//        'rar' => 'Антиквариат и винтаж',
//        'soft' => 'Софт и игры',
//        'summer' => 'Всё для лета!',
//        'tech' => 'Электроника'
    );

    header('Content-Type: text/html; charset=utf-8');

//    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
//        ':provider' => $provider
//    ));

//    db_query("INSERT INTO {wish2wish_categories}
//            VALUES (NULL, :category_id, :category_parent_id, 0, 0, :title, :provider, 0)", array(
//        ':category_id' => 'ozon',
//        ':category_parent_id' => '0',
//        ':title' => 'Озон',
//        ':provider' => 'ozon.ru'
//    ));

    foreach ($catalogs_to_parse as $catalog_to_parse => $catalog_title) {

        $category_id = 'ozon_'.$catalog_to_parse;

        db_query("INSERT INTO {wish2wish_categories}
            VALUES (NULL, :category_id, :category_parent_id, 0, 0, :title, :provider, 1)", array(
                ':category_id' => $category_id,
                ':category_parent_id' => 'ozon',
                ':title' => $catalog_title,
                ':provider' => $provider
            ));

        $reader = new XMLReader();
        $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/ozon/div_'.$catalog_to_parse.'.xml');

        $document = new DOMDocument();

        while ($reader->read() && $reader->name !== 'category');

        while ($reader->name === 'category') {

            $node = simplexml_import_dom($document->importNode($reader->expand(), true));

            db_query("INSERT INTO {wish2wish_categories}
                VALUES (NULL, :category_id, :category_parent_id, 0, 0, :title, :provider, 0)", array(
                    ':category_id' => 'ozon_'.$catalog_to_parse.'_'.(String)$node->attributes()->id,
                    ':category_parent_id' => isset($node->attributes()->parentId) ? 'ozon_'.$catalog_to_parse.'_'.(String)$node->attributes()->parentId : $category_id,
                    ':title' => (String)$node[0],
                    ':provider' => $provider
                ));

            $reader->next('category');

        }

    }


    drupal_set_message('Категории ozon.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_ozon_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    if ($step_param) {
        $catalogs_info = array(
            'book' => 'Книги',
            'appliance' => 'Бытовая техника',
            'beauty' => 'Красота и здоровье',
            'bs' => 'Спорт и отдых',
            'dvd' => 'DVD и Blu-ray',
            'fashion' => 'Одежда, обувь, аксессуары',
            'gifts' => 'Подарки и сувениры',
            'home' => 'Дом, сад, зоотовары',
            'kid' => 'Детям и мамам',
            'music' => 'Музыка',
            'rar' => 'Антиквариат и винтаж',
            'soft' => 'Софт и игры',
            'summer' => 'Всё для лета!',
            'tech' => 'Электроника'
        );
        $catalogs_to_parse = array($step_param => $catalogs_info[$step_param]);
    } else {
        $catalogs_to_parse = array(
//        'book' => 'Книги',
        'appliance' => 'Бытовая техника',
//        'beauty' => 'Красота и здоровье',
//        'bs' => 'Спорт и отдых',
//        'dvd' => 'DVD и Blu-ray',
//        'fashion' => 'Одежда, обувь, аксессуары',
//        'gifts' => 'Подарки и сувениры',
//        'home' => 'Дом, сад, зоотовары',
//        'kid' => 'Детям и мамам',
//        'music' => 'Музыка',
//        'rar' => 'Антиквариат и винтаж',
//        'soft' => 'Софт и игры',
//        'summer' => 'Всё для лета!',
//        'tech' => 'Электроника'
        );
    }

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    header('Content-Type: text/html; charset=utf-8');

    $filter_childrens = array();
    $counter = 0;
    foreach ($catalogs_to_parse as $catalog_to_parse => $catalog_title) {
        $counterFor1Cat = 0;
        $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'ozon'));
        $filters = array();
        foreach ($filters_res as $filter) {
            $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
            $filters[$filter->fid] = $filter;
        }

        $filename = $_SERVER['DOCUMENT_ROOT'].'/data/ozon/div_'.$catalog_to_parse.'_fixed.xml';
        $fileBasename = 'div_'.$catalog_to_parse.'.xml';
        $reader = new XMLReader();
        $reader->open($filename);

        $document = new DOMDocument();

        while ($reader->read() && $reader->name !== 'offer');

        $p_c = variable_get('parser_continue', null);
        if ($p_c) {
            while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

                $total_index++;
                $reader->next('offer');
            }
            $updated = $p_c['updated'];
            variable_del('parser_continue');
        } else {
            db_query("DELETE wfv.* FROM wish2wish_filter_values wfv
             INNER JOIN wish2wish_filter wf ON wf.fid = wfv.fid
             WHERE wf.catalog = '$catalog_to_parse'");
//            _cp_delete_products($filename, $provider);
        }

        while ($reader->name === 'offer') {

            if (time() - $started_time >= 500) {
                print 'No enough time. Schedule task continue.';
                variable_set('parser_continue', array(
                    'total_index' => $total_index,
                    'prefix' => $prefix,
                    'provider' => $provider,
                    'step_param' => $step_param,
                    'updated' => $updated
                ));
                return 'noenoughtime';
                break;
            }

            if ($node_to_expand = $reader->expand()) {
                $node = simplexml_import_dom($document->importNode($node_to_expand, true));

                $item_id = (String)$node->attributes()->id;
                $url = (String)$node->url;
                $price = (String)$node->price;
				$ratio = 0;
				if ($catalog_to_parse == 'tech')
				{
					$ratio = 0.01;
				}
				else
				{
					$ratio = 0.12;
				}
                $currency = (String)$node->currencyId;
                $category = array();
                foreach ($node->categoryId as $key=>$value) {
                    $category[] = (String)$value;
                }
                if (0 && in_array('1134296', $category)) {
                    print '<pre>'.print_r($node, 1).'</pre>';
                } else {
                    $pictures = array();
                    foreach ($node->picture as $key=>$value) {
                        $pictures[] = (String)$value;
                    }
                    //name, title, model+vendor
                    if ($node->name) {
                        $description = (String)$node->name;
                    } elseif($node->title) {
                        $description = (String)$node->title;
                    } elseif ($node->model && $node->vendor) {
                        $description = (String)$node->model.' '.(String)$node->vendor;
                    }
                    $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
                    $vendor = (String)$node->vendor;
                    $model = $node->vendor && $node->vendorCode ? (String)$node->vendor.(String)$node->vendorCode : 'ozon-'.$catalog_to_parse.'-'.$item_id;
                    $params = array();
                    foreach ($node->param as $key=>$value) {
                        $params[(String)$value->attributes()->name] = (String)$value;
                    }
                    $group_item_id = 0;

                    $delivery = (String)$node->delivery;
                    $full_description = $node->description ? (String)$node->description : '';

                    if ($url && $price && $currency && !empty($category) && $description) {
                        $item = array();
                        $item['url'] = $url;
                        $item['price'] = $price;
                        $item['currency'] = $currency;
                        $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                        $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                        $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                        $item['picture'] = '';
                        if (!empty($pictures)) {
                            $item['picture'] = $pictures[0];
                            $item['data']['pictures'] = $pictures;
                        }
                        $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                        $item['data']['vendor'] = $item['vendor'];
                        $item['data']['params_names'] = $params;
                        if ($full_description) {
                            $item['data']['full_description'] = $full_description;
                        }
                        $item['available'] = $available;
                        $item['item_id'] = $item_id;
                        $item = (object) $item;

                        $stats['totalCount']++;

                        if (count($params)) {
                            $stats['withParams']++;
                        }

                        $picturesCount = count($pictures);
                        if ($picturesCount > 1) {
                            $stats['moreThan1img']++;
                        } elseif($picturesCount == 0) {
                            $stats['noImg']++;
                        }

                        foreach ($category as $cat) {
                            $_category_id = $prefix.'_'.$cat;
                            if ($available) {
                                foreach ($filters as $key => $f) {
                                    if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                        $value_exists = null;
                                        if (isset($params[$f->name])) {
                                            $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                                ':model' => $item->model,
                                                ':value' => $params[$f->name],
                                                ':cid' => $prefix.'_'.$cat
                                            ))->fetchField();
                                        }

                                        if (isset($params[$f->name]) && !$value_exists) {
                                            db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                                ':fid' => $f->fid,
                                                ':model' => $item->model,
                                                ':value' => $params[$f->name],
                                                ':cid' => $prefix.'_'.$cat,
                                                ':fmn' => $f->machine_name,
                                                ':f' => $fileBasename
                                            ));
                                        }
                                        if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                            db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                                ':fid' => $f->fid,
                                                ':model' => $item->model,
                                                ':value' => $item->vendor,
                                                ':cid' => $prefix.'_'.$cat,
                                                ':fmn' => $f->machine_name,
                                                ':f' => $fileBasename
                                            ));
                                        }
                                    }
                                }
                            }
                            $counter++;
                            $counterFor1Cat++;

                            if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                                db_query("UPDATE {wish2wish_products}
                                SET
                                category_id = :category_id,
                                url = :url,
                                price = :price,
                                currency = :currency,
                                model = :model,
                                vendor = :vendor,
                                picture = :picture,
                                data = :data,
                                provider = :provider,
                                type_prefix = :descr,
                                group_id = :gid,
                                item_id = :iid,
                                search = :search,
                                type_vendor = :type_vendor,
                                available = :available,
                                filename = :filename,
                                url_hash = :urlHash,
                                updated = :updated
                                WHERE id = :id", array(
                                    ':category_id' => $prefix.'_'.$cat,
                                    ':url' => $item->url,
                                    ':price' => $item->price,
                                    ':currency' => $item->currency,
                                    ':model' => $item->model,
                                    ':vendor' => $item->vendor,
                                    ':picture' => $item->picture,
                                    ':data' => drupal_json_encode($item->data),
                                    ':provider' => $provider,
                                    ':descr' => $item->description,
                                    ':gid' => 0,
                                    ':iid' => $item_id,
                                    ':id' => $id,
                                    ':search' => mb_strlen($item->description.' '.$item->model) > 255 ? mb_substr($item->description.' '.$item->model, 0, 255) : $item->description.' '.$item->model,
                                    ':type_vendor' => mb_strlen($item->description.' '.$item->vendor) > 255 ? mb_substr($item->description.' '.$item->vendor, 0, 255) : $item->description.' '.$item->vendor,
                                    ':available' => $item->available,
                                    ':filename' => $fileBasename,
                                    ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                                    ':updated' => $updated
                                ));
                            } else {
                                db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                                    ':category_id' => $prefix.'_'.$cat,
                                    ':url' => $item->url,
                                    ':price' => $item->price,
                                    ':currency' => $item->currency,
                                    ':model' => $item->model,
                                    ':vendor' => $item->vendor,
                                    ':picture' => $item->picture,
                                    ':data' => drupal_json_encode($item->data),
                                    ':type_prefix' => $item->description,
                                    ':provider' => $provider,
                                    ':gid' => 0,
                                    ':iid' => $item_id,
                                    ':search' => mb_strlen($item->description.' '.$item->model) > 255 ? mb_substr($item->description.' '.$item->model, 0, 255) : $item->description.' '.$item->model,
                                    ':type_vendor' => mb_strlen($item->description.' '.$item->vendor) > 255 ? mb_substr($item->description.' '.$item->vendor, 0, 255) : $item->description.' '.$item->vendor,
                                    ':available' => $item->available,
                                    ':ratio' => 0,
                                    ':filename' => $fileBasename,
                                    ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                                    ':updated' => $updated
                                ));
                            }
                        }
                    }
                }
            }
            $total_index++;
            $reader->next('offer');
        }

        if($counterFor1Cat) {
            print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
            print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
        }

    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}


function _cp_neopod_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'neopod',
        ':category_parent_id' => '0',
        ':title' => 'Neopod',
        ':provider' => 'neopod.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'neopod_main',
        ':category_parent_id' => 'neopod',
        ':title' => 'Игрушки',
        ':provider' => 'neopod.ru'
    ));

    $xml_data = file_get_contents('http://www.neopod.ru/Neopod.ru/info/dealers/xml/get.jsp?refid=www.wish2wish.ru');
    if ($xml_data) {
        unlink($_SERVER['DOCUMENT_ROOT'].'/data/neopod/neopod.xml');
        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/neopod/neopod.xml', 'w+');
        fputs($f, $xml_data);
        fclose($f);
    }

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/neopod/neopod.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'neopod_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'neopod_'.(String)$node->attributes()->parentId : 'neopod_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

        $reader->next('category');

    }


    drupal_set_message('Категории neopod.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_neopod_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'neopod'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }

	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/neopod/neopod.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'neopod.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $cart_url = 'cart-url';
            $url = (String)$node->$cart_url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'neopod-'.$item_id;
            $search = $node->model ? (String)$node->model : '';
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $params = array();
            $ratio = 0.05;
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;
                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->description,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->description,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;;
}


function _cp_4glaza_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => '4glaza',
        ':category_parent_id' => '0',
        ':title' => '4glaza',
        ':provider' => '4glaza.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => '4glaza_main',
        ':category_parent_id' => '4glaza',
        ':title' => 'Оптическая техника',
        ':provider' => '4glaza.ru'
    ));

    /*$xml_data = file_get_contents('http://www.4glaza.ru/4glaza.ru/info/dealers/xml/get.jsp?refid=www.wish2wish.ru');
    if ($xml_data) {
        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/4glaza/4glaza.xml');
        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/4glaza/4glaza.xml', 'w+');
        fputs($f, $xml_data);
        fclose($f);
    }*/

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/4glaza/4glaza.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => '4glaza_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? '4glaza_'.(String)$node->attributes()->parentId : '4glaza_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

        $reader->next('category');

    }


    drupal_set_message('Категории 4glaza.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_4glaza_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => '4glaza'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }


	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/4glaza/4glaza.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = '4glaza.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            //$id_array = explode('-', $item_id);
            //$item_id = $id_array[0];

            $cart_url = 'cart-url';
            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = '4glaza-'.$item_id;
            $search = $node->model ? (String)$node->model : '';
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $params = array();
            $ratio = 0.150;
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;
                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }

                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->description,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->description,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;;
}

function _cp_rctoday_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'rctoday',
        ':category_parent_id' => '0',
        ':title' => 'rctoday',
        ':provider' => 'rc-today.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'rctoday_main',
        ':category_parent_id' => 'rctoday',
        ':title' => 'Радиоуправляемые модели',
        ':provider' => 'rc-today.ru'
    ));

    $xml_data = file_get_contents('http://rc-today.ru/yml/yandex.php');
    if ($xml_data) {
        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/rctoday/rctoday.xml');
        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/rctoday/rctoday.xml', 'w+');
        fputs($f, $xml_data);
        fclose($f);
    }

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/rctoday/rctoday.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'rctoday_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'rctoday_'.(String)$node->attributes()->parentId : 'rctoday_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

        $reader->next('category');

    }


    drupal_set_message('Категории rc-today.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_rctoday_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'rctoday'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }

//////////////////////////////////////////////////CSV////////////////////////////////////////////////////////////
//парсинг csv файла и запись в массив
$resultCSV = array();
$handle = fopen($_SERVER['DOCUMENT_ROOT'].'/data/rctoday/base.csv', "r");
while (!feof($handle))
{
	$data = fgets($handle);
	$str = explode(';', $data);

    if (count($str) > 1) {
        $utf_key = iconv('windows-1251', 'utf-8', $str[1]);
        $key = mb_substr($utf_key, 1, mb_strlen($utf_key) - 2);
        $resultCSV[$key]['description'] = $str[4] ? mb_substr($str[4], 1, mb_strlen($str[4]) - 2) : '';
        $resultCSV[$key]['img'] = '//rc-today.ru'.$str[5];
    }
}

fclose($handle);
$sizeCSV = sizeof($resultCSV);
//$csvNode = db_select('wish2wish_products', 'n');
//$csvNode->fields('n', array('search', 'data'));
//$csvNode->condition('n.provider', 'rc-today.ru');
//$csvNodeQuery = $csvNode->execute();
//while ($csvRecord = $csvNodeQuery->fetchAssoc())
//{
//	if (isset($resultCSV[$csvRecord['search']]))
//	{
//		$data = drupal_json_decode($csvRecord['data']);
//		$data['full_description'] = $resultCSV[$csvRecord['search']]['description'];
//		$updateData = drupal_json_encode($data);
//
//		$updateRecordData = db_update('wish2wish_products');
//		$updateRecordData->fields(array('data' => $updateData));
//		$updateRecordData->condition('search', $resultCSV[$csvRecord['search']]);
//		$updateNewRecord = $updateRecordData->execute();
//	}
//}
///////////////////////////////////////////////////CSV////////////////////////////////////////////////////////////
	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/rctoday/rctoday.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'rctoday.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'rctoday-'.$item_id;
            $search = $node->name ? (String)$node->name : '';
			$nameForCSV = $search;
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $params = array();
            $ratio = 0;
			if (preg_match('/Бензиновый детский квадроцикл((\w)*(\s)*(-)*)*2T((\w)*(\s)*(-)*)*/ui', (String)$node->name))
			{
				$ratio = 0.07;
			}
			else
			{
				$ratio = 0.1;
			}
			
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;
                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';

                $db_item = db_select('rctoday_products', 'rp')
                    ->fields('rp')
                    ->condition('name', $nameForCSV ? $nameForCSV : '---')
                    ->execute()
                    ->fetchObject();

                if ($db_item && !empty($db_item->content)) {
                    $item['data']['full_description'] = $db_item->content;
                } else {
                    $item['data']['full_description'] = $full_description;
                }

                $_p = array();
                if ($db_item) {
                    $db_images = db_select('rctoday_foto', 'rf')
                        ->fields('rf')
                        ->condition('parent', $db_item->id)
                        ->execute()
                        ->fetchAll();

                    if (!empty($db_item->pic_big) && preg_match('/.+\.(?:jpg|jpeg|png|gif)/', $db_item->pic_big)) {
                        $_p[] = preg_replace('/(.+)\.((?:jpg|jpeg|png|gif))/', 'http://rc-today.ru$1_big.$2', $db_item->pic_big);
                    }

                    if (!empty($db_images)) {
                        foreach ($db_images as $db_image) {
                            if (preg_match('/.+\.(?:jpg|jpeg|png|gif)/', $db_image->name) && $db_item->pic_big != $db_image->name) {
                                $_p[] = preg_replace('/(.+)\.((?:jpg|jpeg|png|gif))/', 'http://rc-today.ru$1_big.$2', $db_image->name);
                            }
                        }
                    }
                }

                if (!empty($_p)) {
                    $item['picture'] = $_p[0];
                    $item['data']['pictures'] = $_p;
                } else {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }

                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;

                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;

                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }

                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->description,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->description,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;;
}

function _cp_sledopyt_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'sledopyt',
        ':category_parent_id' => '0',
        ':title' => 'sledopyt',
        ':provider' => 'sledopyt.su'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'sledopyt_main',
        ':category_parent_id' => 'sledopyt',
        ':title' => 'Для охотника и рыболова',
        ':provider' => 'sledopyt.su'
    ));

    $xml_data = file_get_contents('http://sledopyt.su/bitrix/catalog_export/yandex_423887.php');
    if ($xml_data) {
        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/sledopyt/sledopyt.xml');
        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/sledopyt/sledopyt.xml', 'w+');
        fputs($f, $xml_data);
        fclose($f);
    }

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/sledopyt/sledopyt.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'sledopyt_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'sledopyt_'.(String)$node->attributes()->parentId : 'sledopyt_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

        $reader->next('category');

    }


    drupal_set_message('Категории sledopyt.su перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_sledopyt_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'sledopyt'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }


    $xml_data = file_get_contents('http://sledopyt.su/bitrix/catalog_export/yandex_423887.php');
    if ($xml_data) {
        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/sledopyt/sledopyt.xml');
        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/sledopyt/sledopyt.xml', 'w+');
        fputs($f, $xml_data);
        fclose($f);
    }
	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/sledopyt/sledopyt.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();
/////////////////////////////////////////////////////////////////////////////////////////////////////////
	$arrayGunz = array();
	while ($reader->read() && $reader->name !== 'category');
	while ($reader->name === 'category') 
	{
		$node = simplexml_import_dom($document->importNode($reader->expand(), true));
		if ((string)$node->attributes()->id != 1957)
		{
			$arrayGunz[] = (string)$node->attributes()->id;
			$reader->next('category');
		}
		else
		{
			break;
		}
        //$reader->next('category');
    }
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'sledopyt.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));
			if (in_array((string)$node->categoryId, $arrayGunz))
			{
				goto a;
			}
			$item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'sledopyt-'.$item_id;
            $search = $node->name ? (String)$node->name : '';
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $params = array();
            $ratio = 0.10;
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;
                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->description,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->description,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        a: $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;;
}

function _cp_bestbijoux_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'bestbijoux',
        ':category_parent_id' => '0',
        ':title' => 'bestbijoux',
        ':provider' => 'bestbijoux.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'bestbijoux_main',
        ':category_parent_id' => 'bestbijoux',
        ':title' => 'Модная бижутерия',
        ':provider' => 'bestbijoux.ru'
    ));

    $xml_data = file_get_contents('http://www.bestbijoux.ru/feeds/butikus_goods.xml');
    if ($xml_data) {
        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/bestbijoux/bestbijoux.xml');
        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/bestbijoux/bestbijoux.xml', 'w+');
        fputs($f, $xml_data);
        fclose($f);
    }

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/bestbijoux/bestbijoux.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'bestbijoux_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'bestbijoux_'.(String)$node->attributes()->parentId : 'bestbijoux_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

        $reader->next('category');

    }


    drupal_set_message('Категории bestbijoux.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_bestbijoux_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'bestbijoux'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }


//    $xml_data = file_get_contents('http://www.bestbijoux.ru/feeds/butikus_goods.xml');
//    if ($xml_data) {
//        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/bestbijoux/bestbijoux.xml');
//        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/bestbijoux/bestbijoux.xml', 'w+');
//        fputs($f, $xml_data);
//        fclose($f);
//    }
	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/bestbijoux/bestbijoux.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'bestbijoux.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'bestbijoux-'.$item_id;
            $search = $node->name ? (String)$node->name : '';

//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $params = array();
            $ratio = 0.150;
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;
                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';
            $full_description = preg_replace(array(
                '/\—/isu',
                '/Страна производитель/isu'
            ), array(
                '<br />&nbsp;&nbsp;&nbsp;&nbsp;—',
                '<br />Страна производитель'
            ), $full_description);

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    $title = preg_replace('/Модель[^<]*/isu', '', $search);
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $title,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $title,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $title,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $title,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}

function _cp_cosmotop_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'cosmotop',
        ':category_parent_id' => '0',
        ':title' => 'cosmotop',
        ':provider' => 'cosmotop.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'cosmotop_main',
        ':category_parent_id' => 'cosmotop',
        ':title' => 'Косметика cosmotop',
        ':provider' => 'cosmotop.ru'
    ));

    $xml_data = file_get_contents('http://cosmotop.ru/serv_yml.php');
    if ($xml_data) {
        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/cosmotop/cosmotop.xml');
        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/cosmotop/cosmotop.xml', 'w+');
        fputs($f, $xml_data);
        fclose($f);
    }

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/cosmotop/cosmotop.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'cosmotop_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'cosmotop_'.(String)$node->attributes()->parentId : 'cosmotop_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

        $reader->next('category');

    }


    drupal_set_message('Категории cosmotop.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_cosmotop_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'cosmotop'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }


	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/cosmotop/cosmotop.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'cosmotop.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'cosmotop-'.$item_id;
            $search = (String)$node->model. ' ' . $vendor;
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $params = array();
            $ratio = 0.05;
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;
                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->description,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->description,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}

function _cp_iconv_1251_utf($text) {
    return iconv('windows-1251', 'utf-8', $text);
}

function _cp_iconv_utf_1251($text) {
    return iconv('utf-8', 'windows-1251', $text);
}

//**************************************************************************************************
//**************************Загрзка evoi.ru********************************************************
function _cp_evoi_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'evoi',
        ':category_parent_id' => '0',
        ':title' => 'evoi',
        ':provider' => 'evoi.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'evoi_main',
        ':category_parent_id' => 'evoi',
        ':title' => 'Впечатления',
        ':provider' => 'evoi.ru'
    ));

    _cp_load_data($prefix, 'www.evoi.ru', '/catalog_msk.xml?nostriptags');
	 $file = file_get_contents($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml');
    $file = str_replace('windows-1251', 'utf-8', $file);
    file_put_contents($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml', $file);
	
    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    error_reporting(0);
    ini_set('display_errors', 0);
    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'evoi_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'evoi_'.(String)$node->attributes()->parentId : 'evoi_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

//        drupal_set_message(detect_encoding((String)$node[0]));

        $reader->next('category');

    }


    drupal_set_message('Категории evoi.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_evoi_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'evoi'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }


    $file = file_get_contents($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml');
    $file = str_replace('windows-1251', 'utf-8', $file);
    file_put_contents($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml', $file);


	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'evoi.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $params = array();
            $ocassions = (Array)$node->occasionList->occasion;
            foreach ($ocassions as $occasion) {
                if (is_string($occasion)) {
                    $params['Повод'][] = $occasion;
                }
            }
            $forwhoms = (Array)$node->forwhomList->forwhom;
            foreach ($forwhoms as $forwhom) {
                if (is_string($forwhom)) {
                    $params['Для кого'][] = $forwhom;
                }
            }


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }
//pictures
			$pictures = array();
			$pictureNode = simplexml_import_dom($node->picturesList);
			$picture = $pictureNode->picture;
			$countPicture = sizeof($picture);
			for ($i = 0; $i < $countPicture; $i++)
			{
				$pictures[] = (string)$picture[$i];
			}	
//            $pictures = array();
//            foreach ($node->picture as $key=>$value) {
//                $pictures[] = (String)$value;
//            }
            $description = 'Подарок-впечатление '.(String)$node->name;
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = '';
            $model = 'evoi-'.$item_id;
            $search = 'Подарок-впечатление '.(String)$node->name. ' ' . $vendor;
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
//            $params = array();
            $ratio = 0.10;

            $group_item_id = 0;

            $delivery = (String)$node->delivery;
//full_description//////////////////////////////////////////////////			
			$full_description = '';
		    $description = 'Подарок-впечатление '.(String)$node->name;
           	if ($node->description)
			{
				$descriptionStart = htmlspecialchars_decode((string)$node->description);
				$descriptions = preg_replace('/<a[^>]*>(.*)<\/a>/isU', '$1', $descriptionStart);
				$full_description = $descriptions;
			}	
			if ($node->program)
			{
				$programStart = htmlspecialchars_decode((string)$node->program);
				$program = preg_replace('/<a[^>]*>(.*)<\/a>/isU', '$1',(string)$programStart);
				$full_description.= ' '.$program;
			}	
//programList
			if ($node->programList)
			{
				$stringPrograms = '';
				$programNode = simplexml_import_dom($node->programList);
				$programs = $programNode->program;
				$countProgram = sizeof($programs);
				for ($k = 0; $k < $countProgram; $k++)
				{
					$text = preg_replace('/<a[^>]*>(.*)<\/a>/isU', '$1', 
										htmlspecialchars_decode((string)$programs[$k]));
					//$text = preg_replace('/<|&amp;lt;a[^>|&amp;gt;]*>|&amp;gt;(.*)<|&amp;lt;\/a>|&amp;gt;/isU', '$1', (string)$programs[$k]);					
					$stringPrograms.= '<strong>'.(string)$programs[$k]->attributes()->name.'</strong><br/>
									   <img src ="'.(string)$programs[$k]->attributes()->image.'"/><br/>'.
									  $text.'<br/>';
				}	
				
				$full_description.= $stringPrograms;		
			}
            $_params = array();
            foreach ($params as $_key => $_value) {
                if (is_array($_value)) {
                    $_params[$_key] = implode(',<br />', $_value);
                } else {
                    $_params[$_key] = $_value;
                }
            }
//            print '<pre>'.print_r($_params, 1).'</pre>';
//            die();

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $_params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->description,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->description,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}


//**************************************************************************************************
//**************************Загрзка confael.ru********************************************************
function _cp_confael_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'confael',
        ':category_parent_id' => '0',
        ':title' => 'confael',
        ':provider' => 'confael.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'confael_main',
        ':category_parent_id' => 'confael',
        ':title' => 'Шоколад',
        ':provider' => 'confael.ru'
    ));

//    $xml_data = file_get_contents('http://www.evoi.ru/catalog_msk.xml');
//    if ($xml_data) {
//        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml');
//        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml', 'w+');
//        fputs($f, $xml_data);
//        fclose($f);
//    }

    _cp_load_data($prefix, 'www.confaelshop.ru', '/yml/shop.yml');

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/confael/confael.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    error_reporting(0);
    ini_set('display_errors', 0);
    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'confael_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'confael_'.(String)$node->attributes()->parentId : 'confael_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

//        drupal_set_message(detect_encoding((String)$node[0]));

        $reader->next('category');

    }


    drupal_set_message('Категории confael.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_confael_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'confael'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }

    $p_c = variable_get('parser_continue', null);

    if (!$p_c) {
        $simbols = array('qout', '&";');
        $simbolsReplase = array('-');
        $fget = fopen($_SERVER['DOCUMENT_ROOT'].'/data/confael/confael.xml', "r");
        $fset = fopen($_SERVER['DOCUMENT_ROOT'].'/data/confael/confael_new.xml', 'w');
        if ($fget)
        {
            while (!feof($fget))
            {
                $string = fgets($fget, 1024);
                $string = str_replace($simbols, $simbolsReplase, $string);
                fwrite($fset, $string);
            }
            fflush($fset);
            fclose($fset);
            fclose($fget);
        }
    }


	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/confael/confael_new.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'confael_new.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'confael-'.$item_id;
            $search = (String)$node->name;
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $params = array();
            $ratio = 0.07;
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;
                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }

                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $search,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $search,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}

//////////////////////////////////////////////FIRST-STORE.RU////////////////////////////////////////////////////////////////
function _cp_firststore_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'firststore',
        ':category_parent_id' => '0',
        ':title' => 'firststore',
        ':provider' => 'first-store.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'firststore_main',
        ':category_parent_id' => 'firststore',
        ':title' => 'Аксесуары для Apple',
        ':provider' => 'first-store.ru'
    ));

//    $xml_data = file_get_contents('http://www.evoi.ru/catalog_msk.xml');
//    if ($xml_data) {
//        @unlink($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml');
//        $f = fopen($_SERVER['DOCUMENT_ROOT'].'/data/evoi/evoi.xml', 'w+');
//        fputs($f, $xml_data);
//        fclose($f);
//    }

    _cp_load_data($prefix, 'www.first-store.ru', '/partner_export.php');

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/firststore/firststore.xml');

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'category');

    error_reporting(0);
    ini_set('display_errors', 0);
    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'firststore_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'firststore_'.(String)$node->attributes()->parentId : 'firststore_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

//        drupal_set_message(detect_encoding((String)$node[0]));

        $reader->next('category');

    }


    drupal_set_message('Категории first-store.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_firststore_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'firststore'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }
//    _cp_load_data('firststore', 'www.first-store.ru', '/partner_export.php');

	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/firststore/firststore.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'firststore.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'firststore-'.$item_id;
            $search = (String)$node->name;
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $ratio = 0.07;
			
//param	
			$params = array();		
			foreach ($node->param as $key => $value)
			{
            	$params[(String)$value->attributes()->name] = (String)$value;
            }

            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $search,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $search,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}

///////////////////////////////////////////////PLEER.RU/////////////////////////////////////////////////////////////////
function _cp_pleer_parser($prefix, $provider) {
	
    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'pleer',
        ':category_parent_id' => '0',
        ':title' => 'pleer',
        ':provider' => 'pleer.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'pleer_main',
        ':category_parent_id' => 'pleer',
        ':title' => 'Фото, аксессуары, студийное оборудование',
        ':provider' => 'pleer.ru'
    ));
	//_cp_load_data($prefix, 'www.pleer.ru', '/yafwefdsfere.php?atl=wish2wish');
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	$reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/pleer/pleer.xml');
    $document = new DOMDocument();
	$parentFotoId = '';
    while ($reader->read() && $reader->name !== 'category');
	while ($reader->name === 'category')
	{
		$node = simplexml_import_dom($document->importNode($reader->expand(), true));
		if (preg_match('/Фото, аксессуары, студийное оборудование/u', (string)$node[0]))
		{
			$parentFotoId = (string)$node->attributes()->id;
			break;
		}
		else
		{
			$reader->next('category');
		}	
    }	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while ($reader->name === 'category')
	{
		$node = simplexml_import_dom($document->importNode($reader->expand(), true));
		if ((string)$node->attributes()->parentId == $parentFotoId)
		{
			db_query("INSERT INTO {wish2wish_categories}
           	(category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
           	  		':category_id' => 'pleer_'.(String)$node->attributes()->id,
            	    ':category_parent_id' => 'pleer_main',
            	    ':title' => (string)$node[0],
            	    ':provider' => $provider));
		}
		$reader->next('category');
    }	
	drupal_set_message('Категории pleer.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_pleer_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'pleer'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }

    //_cp_load_data('pleer', 'www.pleer.ru', '/yafwefdsfere.php?atl=wish2wish');

	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/pleer/pleer.xml';
    $reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'pleer.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));


            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url.'&utm_source=wish2wish&utm_medium=cpa&utm_campaign=paid_order';
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'pleer-'.$item_id;
            $search = (String)$node->name;
//            $model = $node->model ? (String)$node->model : 'neopod-'.$item_id;
            $params = array();
            $ratio = 0;
			if ((String)$node->categoryId == 72)
			{
				$ratio = 0.015;
			}
			if ((String)$node->categoryId == 239)
			{
				$ratio = 0.08;
			}
            foreach ($node->param as $key=>$value) {
                $p_name = (String)$value->attributes()->name;
                if (!in_array($p_name, array('commission', 'vendor_url'))) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                } else {
                    if ($p_name == 'commission') {
                        $ratio = (String)$value;
                    }
                }

            }
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $search,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $search,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $search,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}

//////////////////////////////////////////////////////VESTA-ALPHA.RU/////////////////////////////////////////////////////
function _cp_vestaalpha_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    $catalogs_to_parse = array(
        	'podarki' => 'Подарки',
        	//'suvenirka' => 'Сувениры',
        	'uvelirka' => 'Ювелирные изделия',
        	//'christmas' => 'Новогодняя продукция',
        	//'elite-line' => 'Одежда для промоушн и аксессуары',
        	//'promo' => 'Промо'
    );

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

   db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'vestaalpha_main',
        ':category_parent_id' => 'vestaalpha',
        ':title' => 'Лекадо',
        ':provider' => 'vesta-alpha.com'
    ));

	 db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'vestaalpha',
        ':category_parent_id' => '0',
        ':title' => 'Лекадо',
        ':provider' => 'vesta-alpha.com'
    ));

    foreach ($catalogs_to_parse as $catalog_to_parse => $catalog_title) {

        $category_id = 'vestaalpha_'.$catalog_to_parse;
			       		
		db_query("INSERT INTO {wish2wish_categories}
            			(category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            	VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        				':category_id' => $category_id,
        				':category_parent_id' => 'vestaalpha_main',
        				':title' => $catalog_title,
        				':provider' => 'vesta-alpha.com'
    ));
		 _cp_load_data($prefix, 'www.vesta-alpha.com', '/'.$catalog_to_parse.'.xml', $catalog_to_parse);

        $reader = new XMLReader();
        $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/vestaalpha/div_'.$catalog_to_parse.'.xml');

        $document = new DOMDocument();

        while ($reader->read() && $reader->name !== 'category');

        while ($reader->name === 'category') {

            $node = simplexml_import_dom($document->importNode($reader->expand(), true));
           			
			db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'vestaalpha_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parent) ? 'vestaalpha_'.
			(String)$node->attributes()->parent : 'vestaalpha_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));
	
            $reader->next('category');
        }
    }

    drupal_set_message('Категории vesta-alpha.com перестроены.');
    drupal_goto('parser');
    return 'Done';
}

function _cp_vestaalpha_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    $catalogs_to_parse = array(
        'podarki' => 'Подарки',
        //'suvenirka' => 'Сувениры',
        'uvelirka' => 'Ювелирные изделия',
        //'christmas' => 'Новогодняя продукция',
        //'elite-line' => 'Одежда для промоушн и аксессуары',
        //'promo' => 'Промо'
    );

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    foreach ($catalogs_to_parse as $catalog_to_parse => $catalog_title) {
		$filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'vestaalpha'));
        $filters = array();
        foreach ($filters_res as $filter) {
			//var_dump($catalog_to_parse);exit;
            $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
            $filters[$filter->fid] = $filter;
			 
        }
//////////////////////////////////////////////////////////////////////////////////////////////////////////	
//var_dump($filters);
//exit;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
		$filename = $_SERVER['DOCUMENT_ROOT'].'/data/vestaalpha/div_'.$catalog_to_parse.'.xml';	
		$reader = new XMLReader();
        $reader->open($filename);
		$document = new DOMDocument();

        while ($reader->read() && $reader->name !== 'offer');

        $p_c = variable_get('parser_continue', null);
        if ($p_c) {
            while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

                $total_index++;
                $reader->next('offer');
            }
            $updated = $p_c['updated'];
            variable_del('parser_continue');
        } else {
            db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
        }

        $fileBasename = 'div_'.$catalog_to_parse.'.xml';

        while ($reader->name === 'offer') {

            if (time() - $started_time >= 500) {
                print 'No enough time. Schedule task continue.';
                variable_set('parser_continue', array(
                    'total_index' => $total_index,
                    'prefix' => $prefix,
                    'provider' => $provider,
                    'step_param' => $step_param,
                    'updated' => $updated
                ));
                return 'noenoughtime';
                break;
            }

            if ($node_to_expand = $reader->expand()) {
//echo 'expand';
                $node = simplexml_import_dom($document->importNode($node_to_expand, true));

                $item_id = (String)$node->attributes()->id;
				if ((string)$node->url == '')
				{
					$url = 'buy';//(String)$node->url;
				}
				else
				{
					$url = (String)$node->url;
				}	
                $price = (String)$node->price;
                $currency = (String)$node->currencyId;
//params 				
				$params = array();
				$paramsNode = simplexml_import_dom($node->params);
				$param = $paramsNode->param;
				$countParam = sizeof($param);
				for ($i = 0; $i < $countParam; $i++)
				{
					if (preg_match('/Подарки к праздникам/u', (string)$param[$i]->attributes()->name) == 1)
					{
						continue;
					}
					else
					{
						$params[(string)$param[$i]->attributes()->name] = (string)$param[$i];
					}	
				}	
				
				$category = array();
                foreach ($node->categoryId as $key=>$value) {
                    $category[] = (String)$value;
                }
                $pictures = array();
                foreach ($node->picture as $key=>$value) {
                    $pictures[] = (String)$value;
                }
                //name, title, model+vendor
                if ($node->name) {
                    $description = (String)$node->name;
                } elseif($node->title) {
                    $description = (String)$node->title;
                } elseif ($node->model && $node->vendor) {
                    $description = (String)$node->model.' '.(String)$node->vendor;
                }
                $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
                $vendor = (String)$node->vendor;
                $model = $node->vendor && $node->vendorCode ? (String)$node->vendor.(String)$node->vendorCode : 'vestaalpha-'.$catalog_to_parse.'-'.$item_id;
                /*foreach ($node->param as $key=>$value) {
                    $params[(String)$value->attributes()->name] = (String)$value;
                }*/
                $group_item_id = 0;
				$ratio = 0.3;
                $delivery = (String)$node->delivery;
                $full_description = $node->description ? (String)$node->description : '';

//                print '<pre>'.print_r($params, 1).'</pre>';
                if ($price && $currency && !empty($category)) {
                    $item = array();
                    $item['url'] = $url;
                    $item['price'] = $price;
                    $item['currency'] = $currency;
                    $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                    $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                    $item['description'] = mb_strlen($description) > 255 ? mb_substr($description, 0, 255) : $description;
                    $item['picture'] = '';
                    if (!empty($pictures)) {
                        $item['picture'] = $pictures[0];
                        $item['data']['pictures'] = $pictures;
                    }
                    $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                    $item['data']['vendor'] = $item['vendor'];
					$item['data']['params_names'] = $params;
                    if ($full_description) {
                        $item['data']['full_description'] = $full_description;
                    }
                    $item['available'] = $available;
                    $item['item_id'] = $item_id;

                    $stats['totalCount']++;

                    if (count($params)) {
                        $stats['withParams']++;
                    }

                    $picturesCount = count($pictures);
                    if ($picturesCount > 1) {
                        $stats['moreThan1img']++;
                    } elseif($picturesCount == 0) {
                        $stats['noImg']++;
                    }

                    $item = (object) $item;
                    foreach ($category as $cat) {
                        $_category_id = $prefix.'_'.$cat;
                        if ($available) {
                            foreach ($filters as $key => $f) {
                                if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                    $value_exists = null;
                                    if (isset($params[$f->name])) {
                                        $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                            ':model' => $item->model,
                                            ':value' => $params[$f->name],
                                            ':cid' => $prefix.'_'.$cat
                                        ))->fetchField();
                                    }

                                    if (isset($params[$f->name]) && !$value_exists) {
                                        db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                            ':fid' => $f->fid,
                                            ':model' => $item->model,
                                            ':value' => $params[$f->name],
                                            ':cid' => $prefix.'_'.$cat,
                                            ':fmn' => $f->machine_name,
                                            ':f' => $fileBasename
                                        ));
                                    }
                                    if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                        db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                            ':fid' => $f->fid,
                                            ':model' => $item->model,
                                            ':value' => $item->vendor,
                                            ':cid' => $prefix.'_'.$cat,
                                            ':fmn' => $f->machine_name,
                                            ':f' => $fileBasename
                                        ));
                                    }
                                }
                            }
                        }
                        $counter++;
                        if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                            db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                                ':category_id' => $prefix.'_'.$cat,
                                ':url' => $item->url,
                                ':price' => $item->price,
                                ':currency' => $item->currency,
                                ':model' => $item->model,
                                ':vendor' => $item->vendor,
                                ':picture' => $item->picture,
                                ':data' => drupal_json_encode($item->data),
                                ':provider' => $provider,
                                ':descr' => $item->description,
                                ':gid' => 0,
                                ':iid' => $item_id,
                                ':id' => $id,
                                ':search' => mb_strlen($item->description.' '.$item->model) > 255 ? mb_substr($item->description.' '.$item->model, 0, 255) : $item->description.' '.$item->model,
                                ':type_vendor' => mb_strlen($item->description.' '.$item->vendor) > 255 ? mb_substr($item->description.' '.$item->vendor, 0, 255) : $item->description.' '.$item->vendor,
                                ':available' => $item->available,
                                ':ratio' => $ratio,
                                ':filename' => $fileBasename,
                                ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                                ':updated' => $updated
                            ));
                        } else {
                            db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                                ':category_id' => $prefix.'_'.$cat,
                                ':url' => $item->url,
                                ':price' => $item->price,
                                ':currency' => $item->currency,
                                ':model' => $item->model,
                                ':vendor' => $item->vendor,
                                ':picture' => $item->picture,
                                ':data' => drupal_json_encode($item->data),
                                ':type_prefix' => $item->description,
                                ':provider' => $provider,
                                ':gid' => 0,
                                ':iid' => $item_id,
                                ':search' => mb_strlen($item->description.' '.$item->model) > 255 ? mb_substr($item->description.' '.$item->model, 0, 255) : $item->description.' '.$item->model,
                                ':type_vendor' => mb_strlen($item->description.' '.$item->vendor) > 255 ? mb_substr($item->description.' '.$item->vendor, 0, 255) : $item->description.' '.$item->vendor,
                                ':available' => $item->available,
                                ':ratio' => $ratio,
                                ':filename' => $fileBasename,
                                ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                                ':updated' => $updated
                            ));
                        }
                    }
                }
            }

            $total_index++;
            $reader->next('offer');
        }

    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}
//////////////////////////////////////////////	TECHPORT.RU////////////////////////////////////////////////////////////////
function _cp_techport_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'techport',
        ':category_parent_id' => '0',
        ':title' => 'techport',
        ':provider' => 'techport.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'techport_main',
        ':category_parent_id' => 'techport',
        ':title' => 'Бытовая техника',
        ':provider' => 'techport.ru'
    ));

    _cp_load_data($prefix, 'www.techport.ru', '/export_wish2wish.php?city=MSK&rc=wish&from=whishtowhish');
	$file = file_get_contents($_SERVER['DOCUMENT_ROOT'].'/data/techport/techport.xml');
   	$file = str_replace(array(
          'windows-1251'), 
		  array('utf-8'), $file);
    file_put_contents($_SERVER['DOCUMENT_ROOT'].'/data/techport/techport.xml', $file);

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/techport/techport.xml');

    $document = new DOMDocument();
	
	while ($reader->read() && $reader->name !== 'category');
   
    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'techport_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'techport_'.(String)$node->attributes()->parentId : 'techport_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

        $reader->next('category');
    }
    drupal_set_message('Категории techport.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_techport_items_parser ($prefix, $provider, $step_param = null) {
    $started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'techport'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }

	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/techport/techport.xml';
	$reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'techport.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));
			
            $item_id = (String)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'techport-'.$item_id;
			$search = (string)$node->typePrefix ? (String)$node->typePrefix.' '.(String)$node->model : (String)$node->model;
            $type_vendor = (string)$node->typePrefix ? (String)$node->typePrefix.' '.(String)$node->vendor : '';
            			
//param	
			$params = array();		
			foreach ($node->param as $key => $value)
			{
            	$params[(String)$value->attributes()->name] = (String)$value;
            }
//ratio
			if ($params['commission'])
			{
				$ratio = round($params['commission'], 5);
			}
			else
			{
				$ratio = 0;
			}                      
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['type_prefix'] = (string)$node->typePrefix ? (String)$node->typePrefix : '';
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name, filename) VALUES (:fid, :model, :value, :cid, :fmn, :f)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name,
                                        ':f' => $fileBasename
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->type_prefix,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $type_vendor,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->type_prefix,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $type_vendor,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);
    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}


//////////////////////////////////////////////MUZTORG/////////////////////////////////////////////////////////////////
function _cp_muztorg_parser($prefix, $provider) {

    if (!isset($_POST['categories-rebuild'])) {
        ob_start();
        include $_SERVER['DOCUMENT_ROOT'].'/'. drupal_get_path('module', 'cp').'/tpl/cp.categories.rebuild.tpl.php';
        return ob_get_clean();
    }

    header('Content-Type: text/html; charset=utf-8');

    db_query("DELETE FROM {wish2wish_categories} WHERE provider = :provider", array(
        ':provider' => $provider
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'muztorg',
        ':category_parent_id' => '0',
        ':title' => 'muztorg',
        ':provider' => 'muztorg.ru'
    ));

    db_query("INSERT INTO {wish2wish_categories}
            (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
        ':category_id' => 'muztorg_main',
        ':category_parent_id' => 'muztorg',
        ':title' => 'МузТорг',
        ':provider' => 'muztorg.ru'
    ));

    _cp_load_data($prefix, 'www.muztorg.ru', '/muztorg.yml.php');
	/*$file = file_get_contents($_SERVER['DOCUMENT_ROOT'].'/data/muztorg/muztorg.xml');
   	$file = str_replace(array(
          'windows-1251'), 
		  array('utf-8'), $file);
    file_put_contents($_SERVER['DOCUMENT_ROOT'].'/data/muztorg/muztorg.xml', $file);*/

    $reader = new XMLReader();
    $reader->open($_SERVER['DOCUMENT_ROOT'].'/data/muztorg/muztorg.xml');

    $document = new DOMDocument();
	
	while ($reader->read() && $reader->name !== 'category');
   
    while ($reader->name === 'category') {

        $node = simplexml_import_dom($document->importNode($reader->expand(), true));

        db_query("INSERT INTO {wish2wish_categories}
           (category_id, category_parent_id, sex, fid, title, provider, visibility, weight)
            VALUES (:category_id, :category_parent_id, 0, 0, :title, :provider, 0, 0)", array(
            ':category_id' => 'muztorg_'.(String)$node->attributes()->id,
            ':category_parent_id' => isset($node->attributes()->parentId) ? 'muztorg_'.(String)$node->attributes()->parentId : 'muztorg_main',
            ':title' => (String)$node[0],
            ':provider' => $provider
        ));

        $reader->next('category');
    }
    drupal_set_message('Категории muztorg.ru перестроены.');
    drupal_goto('parser');

    return 'Done';
}

function _cp_muztorg_items_parser ($prefix, $provider, $step_param = null) {
	$started = microtime(true);
    $started_time = time();
    $updated = $started_time;
    $output = '';
    $total_index = 0;

    header('Content-Type: text/html; charset=utf-8');

    $stats = variable_get('stats', array(
        'noImg' => 0,
        'moreThan1img' => 0,
        'totalCount' => 0,
        'withParams' => 0
    ));

    $filter_childrens = array();
    $counter = 0;
    $filters_res = db_query("SELECT * FROM wish2wish_filter WHERE catalog = :cat", array(':cat' => 'muztorg'));
    $filters = array();
    foreach ($filters_res as $filter) {
        $filter_childrens[$filter->fid] = array_merge(array($filter->category_id), _events_get_childrens_id($filter));
        $filters[$filter->fid] = $filter;
    }
// 	_cp_load_data($prefix, 'www.muztorg.ru', '/muztorg.yml.php');

	$filename = $_SERVER['DOCUMENT_ROOT'].'/data/muztorg/muztorg.xml';
	$reader = new XMLReader();
    $reader->open($filename);

    $document = new DOMDocument();

    while ($reader->read() && $reader->name !== 'offer');

    $p_c = variable_get('parser_continue', null);
    if ($p_c) {
        while ($reader->name === 'offer' && $total_index < $p_c['total_index']) {

            $total_index++;
            $reader->next('offer');
        }
        $updated = $p_c['updated'];
        variable_del('parser_continue');
    } else {
        db_query("DELETE FROM wish2wish_filter_values WHERE category_id LIKE '".$prefix."_%'");
    }

    $fileBasename = 'muztorg.xml';

    while ($reader->name === 'offer') {

        if (time() - $started_time >= 500) {
            print 'No enough time. Schedule task continue.';
            variable_set('parser_continue', array(
                'total_index' => $total_index,
                'prefix' => $prefix,
                'provider' => $provider,
                'step_param' => $step_param,
                'updated' => $updated
            ));
            return 'noenoughtime';
            break;
        }

        if ($node_to_expand = $reader->expand()) {
            $node = simplexml_import_dom($document->importNode($node_to_expand, true));
			
            $item_id = (int)$node->attributes()->id;
            $id_array = explode('-', $item_id);
            $item_id = $id_array[0];

            $url = (String)$node->url;
            $price = (String)$node->price;
            $currency = (String)$node->currencyId;
            $category = array();
            foreach ($node->categoryId as $key=>$value) {
                $category[] = (String)$value;
            }

            $pictures = array();
            foreach ($node->picture as $key=>$value) {
                $pictures[] = (String)$value;
            }
            $description = '';
            $available = (String)$node->attributes()->available == 'true' ? 1 : 0;
            $vendor = (String)$node->vendor;
            $model = 'muztorg-'.$item_id;
			$search = (String)$node->model;
            $type_vendor = (String)$node->vendor;
            $ratio = 0;
			
//param	
			$params = array();		
			foreach ($node->param as $key => $value)
			{
            	$params[(String)$value->attributes()->name] = (String)$value;
            }
            
            $group_item_id = 0;

            $delivery = (String)$node->delivery;
            $full_description = $node->description ? (String)$node->description : '';

            if ($url && $price && $currency && !empty($category)) {
                $item = array();
                $item['url'] = $url;
                $item['price'] = $price;
                $item['currency'] = $currency;
                $item['model'] = mb_strlen($model) > 255 ? mb_substr($model, 0, 255) : $model;
                $item['vendor'] = mb_strlen($vendor) > 255 ? mb_substr($vendor, 0, 255) : $vendor;
                $item['type_prefix'] = (string)$node->typePrefix ? (String)$node->typePrefix : '';
                $item['picture'] = '';
                if (!empty($pictures)) {
                    $item['picture'] = $pictures[0];
                    $item['data']['pictures'] = $pictures;
                }
                $item['data']['delivery'] = $delivery == 'true' ? 1 : 0;
                $item['data']['vendor'] = $item['vendor'];
                $item['data']['params_names'] = $params;
                if ($full_description) {
                    $item['data']['full_description'] = $full_description;
                }
                $item['available'] = $available;
                $item['item_id'] = $item_id;

                $stats['totalCount']++;

                if (count($params)) {
                    $stats['withParams']++;
                }

                $picturesCount = count($pictures);
                if ($picturesCount > 1) {
                    $stats['moreThan1img']++;
                } elseif($picturesCount == 0) {
                    $stats['noImg']++;
                }

                $item = (object) $item;
                foreach ($category as $cat) {
                    $_category_id = $prefix.'_'.$cat;
                    if ($available) {
                        foreach ($filters as $key => $f) {
                            if (isset($filter_childrens[$f->fid]) && in_array($_category_id, $filter_childrens[$f->fid])) {
                                $value_exists = null;
                                if (isset($params[$f->name])) {
                                    $value_exists = db_query("SELECT id FROM wish2wish_filter_values WHERE model = :model AND value = :value AND category_id = :cid", array(
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat
                                    ))->fetchField();
                                }

                                if (isset($params[$f->name]) && !$value_exists) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name) VALUES (:fid, :model, :value, :cid, :fmn)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $params[$f->name],
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name
                                    ));
                                }
                                if($f->machine_name == 'brand' && !$value_exists && $item->vendor) {
                                    db_query("INSERT INTO wish2wish_filter_values (fid, model, value, category_id, filter_machine_name) VALUES (:fid, :model, :value, :cid, :fmn)", array(
                                        ':fid' => $f->fid,
                                        ':model' => $item->model,
                                        ':value' => $item->vendor,
                                        ':cid' => $prefix.'_'.$cat,
                                        ':fmn' => $f->machine_name
                                    ));
                                }
                            }
                        }
                    }
                    $counter++;
                    if ($id = db_query("SELECT id FROM {wish2wish_products} WHERE category_id = :cid AND item_id = :iid", array(':cid' => $prefix.'_'.$cat, ':iid' => $item_id))->fetchField()) {
                        db_query("UPDATE {wish2wish_products}
                            SET
                            category_id = :category_id,
                            url = :url,
                            price = :price,
                            currency = :currency,
                            model = :model,
                            vendor = :vendor,
                            picture = :picture,
                            data = :data,
                            provider = :provider,
                            type_prefix = :descr,
                            group_id = :gid,
                            item_id = :iid,
                            search = :search,
                            type_vendor = :type_vendor,
                            available = :available,
                            ratio = :ratio,
                            filename = :filename,
                            url_hash = :urlHash,
                            updated = :updated
                            WHERE id = :id", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':provider' => $provider,
                            ':descr' => $item->type_prefix,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':id' => $id,
                            ':search' => $search,
                            ':type_vendor' => $type_vendor,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    } else {
                        db_query("INSERT INTO {wish2wish_products}
                            (category_id, url, price, currency, model, vendor, picture, data, provider, type_prefix, group_id, item_id, search, type_vendor, available, ratio, filename, url_hash, updated)
                            VALUES
                            (:category_id, :url, :price, :currency, :model, :vendor, :picture, :data, :provider, :type_prefix, :gid, :iid, :search, :type_vendor, :available, :ratio, :filename, :urlHash, :updated)", array(
                            ':category_id' => $prefix.'_'.$cat,
                            ':url' => $item->url,
                            ':price' => $item->price,
                            ':currency' => $item->currency,
                            ':model' => $item->model,
                            ':vendor' => $item->vendor,
                            ':picture' => $item->picture,
                            ':data' => drupal_json_encode($item->data),
                            ':type_prefix' => $item->type_prefix,
                            ':provider' => $provider,
                            ':gid' => 0,
                            ':iid' => $item_id,
                            ':search' => $search,
                            ':type_vendor' => $type_vendor,
                            ':available' => $item->available,
                            ':ratio' => $ratio,
                            ':filename' => $fileBasename,
                            ':urlHash' => $item->url ? md5($item->url) : uniqid(),
                            ':updated' => $updated
                        ));
                    }
                }
            }
        }

        $total_index++;
        $reader->next('offer');
    }

    if ($counter) {
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/categories/update?filename=".urlencode($filename)."&provider=".urlencode($provider)."&prefix=".urlencode($prefix)."&cron-task=1' >/dev/null &");
        print system("fetch -o - 'http://".$_SERVER['SERVER_NAME']."/parser/products/remove_old?filename=".urlencode($filename)."&provider=".urlencode($provider)."&updated=".$updated."&cron-task=1' >/dev/null &");
    }

    variable_set('stats', $stats);

    return 'Добавлено: '.number_format($counter, 0, '.', ' ').' за '.number_format(microtime(true) - $started, 2, '.', ' ').'сек. <br />'.$output;
}
