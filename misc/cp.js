function categoryChanged(id) {
    $('.cat-'+id).css('background-color', '#ffd2e5');
    $('.cat-'+id).find('input[type="button"]').show();
}

function categorySave(id) {
    var checked = $('.cat-'+id).find('input[type="checkbox"]').prop('checked') ? 1 : 0;
    var sex = $('.cat-'+id).find('select').val();
    $.ajax('/categories/'+id+'/save', {
        type: 'post',
        dataType: 'json',
        data: {checked: checked, sex: sex},
        success: function(d) {
            if (d && d.result == 1){
                $('.cat-'+id).css('background-color', '');
                $('.cat-'+id).find('input[type="button"]').hide();
            } else {
                alert('Не удалось сохранить изменения. Повторите попытку позже.');
            }
        },
        error: function() {
            alert('Не удалось сохранить изменения. Повторите попытку позже.');
        }
    });
    
    
}

