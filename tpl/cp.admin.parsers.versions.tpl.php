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
    <li class="t1">
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
    <li class="t2 tab-current">
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
                        Дата и время
                    </td>
                    <td>
                        Инфо
                    </td>
                    <td>
                        Утвержден
                    </td>
                    <td>
                        Действия
                    </td>
                </tr>
                <?php $index = 0; ?>
                <?php foreach($versions as $store): ?>
                    <tr<?php if($index%2 == 0): ?> class="LightRow"<?php endif; ?>>
                        <td>
                            <?php echo $store->store_title; ?>
                        </td>
                        <td>
                            <?php echo $store->catalog_title; ?>
                        </td>
                        <td>
                            <?php echo date('d.m.Y H:i', $store->created); ?>
                        </td>
                        <td>
                            <?php $data = unserialize($store->data); ?>
                            <table>
                                <tr>
                                    <td>
                                        Товаров всего:
                                    </td>
                                    <td>
                                        <?php echo $data['totalCount']; ?>
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Товаров у которых нет картинок:
                                    </td>
                                    <td>
                                        <?php echo $data['noImg']; ?>
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Товаров у которых больше чем 1 картинка:
                                    </td>
                                    <td>
                                        <?php echo $data['moreThan1img']; ?>
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Товаров с параметрами:
                                    </td>
                                    <td>
                                        <?php echo $data['withParams']; ?>
                                    </td>
                                </tr>
                            </table>
                        </td>
                        <td>
                            <?php echo $store->approved ? 'Да' : 'Нет'; ?>
                        </td>
                        <td>
                            <?php if(!$store->removed):?>
                                <a href="/parser/versions/<?php echo $store->versionId; ?>/approve">Утвердить</a><br />
                            <?php endif;?>
                            <a href="javascript:;">Удалить</a>
                        </td>
                    </tr>
                    <?php $index++; ?>
                <?php endforeach; ?>
                <!--                <tr class="LightRow">-->
                </tbody></table>
        </div>
    </div>
</div>
</div>
</div>
<div class="MainContainer">