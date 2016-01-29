% include header
<script type="text/javascript" src="/static/js/jquery-1.12.0.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.columnFilter.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>
<script type="text/javascript">
  $(document).ready(function() {
    $('#access').dataTable( {
      "processing": true,
      "ajax": {
        "url": "access_get?id_block={{id_block}}&id_struct={{id_struct}}"
      },
      "createdRow": function( row, data, dataIndex ) {
        if ( data[0] == "<code>write</code>" ) {
	$(row).css('font-weight', 'bold');
  }
  $('td:eq(3)', row).css('font-weight', 'normal');
	},
  % if defined('in_block_view'):
  "columnDefs": [
  { "visible": false, "targets": 4 }
    ]
  % end
  } ).columnFilter({
  aoColumns: [ { type: "select", values : ['read', 'write']},
  { type: "text" },
  { type: "text" },
  { type: "text" },
  null]});
  
  });
</script>
  
<div class="container-fluid">
  <table id="access" class="table table-bordered table-striped">
    <thead>
      <tr>
	<th> access </th>
	<th> offset </th>
	<th> size </th>
	<th> agent </th>
	<th> block_id </th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> access </th>
	<th> offset </th>
	<th> size </th>
	<th> agent </th>
	<th>  </th>
      </tr>
    </tfoot>
  </table>
</div>

% include footer
