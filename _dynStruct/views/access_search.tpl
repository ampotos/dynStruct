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
        if ( data[0] == "read" ) {
          $(row).addClass( 'success' );
        }
        else {
          $(row).addClass( 'danger' );
        }
      },
      "scrollY": 400,
      "scrollCollapse": true
  } ).columnFilter({
  aoColumns: [ { type: "select", values : ['read', 'write']},
  { type: "text" },
  { type: "text" },
  { type: "text" },
  { type: "text" },
  null]});
  } );
</script>
  
<div class="container">
  <table id="access" class="table table-bordered table-striped">
    <thead>
      <tr>
	<th> type </th>
	<th> offset </th>
	<th> size </th>
	<th> instruction pc </th>
	<th> in function </th>
	<th> block link </th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> type </th>
	<th> offset </th>
	<th> size </th>
	<th> instruction pc </th>
	<th> in function </th>
	<th>  </th>
      </tr>
    </tfoot>
  </table>
</div>

% include footer
