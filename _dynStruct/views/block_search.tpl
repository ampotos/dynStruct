% include header
<script type="text/javascript" src="/static/js/jquery-1.12.0.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.columnFilter.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>
<script type="text/javascript">
  $(document).ready(function() {
    $('#blocks').dataTable( {
      "processing": true,
      "ajax": {
        "url": "block_get?id_struct={{id_struct}}"
      },
  % if defined('in_struct_view'):
  "columnDefs": [
  { "visible": false, "targets": 6 }
    ]
  % end
  })}).columnFilter({
  aoColumns: [ { type: "text"},
  { type: "text" },
  { type: "text" },
  { type: "text" },
  { type: "text" },
  { type: "text" },
    null]});
</script>

% if not defined('in_page'):
<div class="container-fluid">
% end
  <table id="blocks" class="table table-bordered" style="width: 100%">
    <thead>
      <tr>
	<th> start </th>
	<th> end </th>
	<th> size </th>
	<th> malloc caller </th>
	<th> free caller </th>
	<th> struct </th>
	<th> detailed view </th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> start </th>
	<th> end </th>
	<th> size </th>
	<th> malloc caller </th>
	<th> free caller </th>
	<th>  </th>
	<th>  </th>
      </tr>
    </tfoot>
  </table>
% if not defined('in_page'):
</div>
% end
% include footer
