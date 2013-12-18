</div>
</div>
</div>
<style>
    .parser-link.disabled, .parser-link.disabled:hover {
        color: #888;
        text-decoration: none;
        cursor: default;
    }
</style>
<div class="AdminContent">
<div class="TabContainer">
<ul class="tabs tabs1">
    <li class="t1 tab-current">
        <div class="TabItem">
            <table>
                <tbody><tr>
                    <td class="AdmTabL">&nbsp;</td>
                    <td class="AdmTabC"><a href="/parser/list">Парсеры</a></td>
                    <td class="AdmTabR">&nbsp;</td>
                </tr>
                </tbody></table>
        </div>
    </li>
    <li class="t2">
        <div class="TabItem">
            <table>
                <tbody><tr>
                    <td class="AdmTabL">&nbsp;</td>
                    <td class="AdmTabC"><a href="/parser/versions">Отчет</a></td>
                    <td class="AdmTabR">&nbsp;</td>
                </tr>
                </tbody></table>
        </div>
    </li>
</ul>
<?php
$storesList = db_select('wish2wish_parser_categories', 'wpc')
    ->fields('wpc')
    ->orderBy('wpc.last_action', 'DESC')
    ->execute();

$continueParsing = false;
?>
<div class="TabContent">
    <div class="t1">
        <div class="line">&nbsp;</div>
        <div class="TableReportBox">
            <table class="TableReport">
                <tbody>
                <tr class="ReportTitle">
                    <td>
                        Магазин
                    </td>
                    <td>
                        Каталог
                    </td>
                    <td>
                        Дата последнего действия
                    </td>
                    <td>
                        Статус
                    </td>
                    <td>
                        Действия
                    </td>
                </tr>
                <?php $index = 0; ?>
                <?php foreach($storesList as $store): ?>
                <tr<?php if($index%2 == 0): ?> class="LightRow"<?php endif; ?>>
                    <td>
                        <?php echo $store->store_title . ' - ' . $store->id; ?>
                    </td>
                    <td>
                        <?php echo $store->catalog_title; ?>
                    </td>
                    <td class="last-action-<?php echo $store->id; ?>">
                        <?php echo date('d.m.Y H:i', $store->last_action); ?>
                    </td>
                    <td class="status-<?php echo $store->id; ?>">
                        <?php
                        $status = "Ошибка";
                        switch ($store->current_status) {
                            case 0:
                                $status = "Завершен";
                                break;
                            case 1:
                                $status = "Загружается каталог";
                                break;
                            case 2:
                                $status = "Каталог загружен. Ожидание.";
                                break;
                            case 3:
                                $status = "Парсинг";
                                break;
                            case 4:
                                $status = "Парсинг";
                                break;
                        }
                        if ($store->current_status > 0) {
                            $continueParsing = true;
                        }
                        ?>
                        <?php echo $status; ?>
                    </td>
                    <td>
                        <a href="javascript:;" class="parser-link parser-link-<?php echo $store->id; ?><?php if($store->current_status > 0):?> disabled<?php endif; ?>" data-item-id="<?php echo $store->id; ?>"><?php echo $store->current_status > 0 ? 'Ожидание' : 'Запустить'; ?></a>
                    </td>
                </tr>
                <?php $index++; ?>
                <?php endforeach; ?>
<!--                <tr class="LightRow">-->
                </tbody></table>
            <?php if($continueParsing): ?>
            <script>
                //parserStep();
            </script>
            <?php endif;?>
        </div>
    </div>
    <script>
        function parserStep(m) {
            $.ajax('/parser/step', {
                type: 'post',
                data: {manual: m ? m : 0},
                success: function() {

                },
                error: function() {

                }
            });
            setTimeout('parserGetStatuses(1)', 200);
        }

        function parserGetStatuses(a) {
            $.ajax('/parser/statuses', {
                type: 'post',
                dataType: 'json',
                success: function(d) {
                    if (d && d.parsers && d.parsers.length) {
                        var working = 0;
                        for (var i =0; i < d.parsers.length; i++) {
                            var id = d.parsers[i].id,
                                status = d.parsers[i].status;

                            if (status > 0) {
                                if (status == 2 || status == 4) {
                                    parserStep(id);
                                }
                                working = id;
                            } else {
                            }

                            $('.parser-link-' + id)
                                .text(d.parsers[i].linkText);

                            $('.last-action-' + id)
                                .text(d.parsers[i].lastAction);

                            $('.status-' + id)
                                .text(d.parsers[i].statusText);
                        }
                        if (working) {
                            $('.parser-link').addClass('disabled');
                        } else {
                            $('.parser-link').removeClass('disabled');
                        }
                    }
                },
                error: function() {

                }
            });

            if (!a)
                setTimeout('parserGetStatuses()', 10000);
        }

        $('.parser-link').click(function() {
            if ($(this).hasClass('disabled')) {
                return;
            }
            var id = $(this).attr('data-item-id');
            parserStep(id);
        });

        $(document).ready(function() {
            parserGetStatuses();
        });
    </script>
</div>
</div>
</div>
<div class="MainContainer">