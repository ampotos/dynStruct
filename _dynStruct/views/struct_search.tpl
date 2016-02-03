% include header
<script type="text/javascript" src="/static/js/jquery-1.12.0.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.columnFilter.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>
<script type="text/javascript">
  $(document).ready(function() {
    $('#structs').dataTable( {
      "processing": true,
      "ajax": {
        "url": "struct_get"
      },
  })}).columnFilter({
  aoColumns: [ { type: "text"}, 
  { type: "text" },
  { type: "text" },
  { type: "text" }]});
</script>

<div class="container-fluid">
  <h1 class="text-center"> Structure search</h1>
  <table id="structs" class="table table-bordered" style="width: 100%">
    <thead>
      <tr>
	<th> name </th>
	<th> size </th>
	<th> nb of members </th>
	<th> nb of instances </th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> name </th>
	<th> size </th>
	<th> nb of members </th>
	<th> nb of instances </th>
      </tr>
    </tfoot>
  </table>
</div>
% include footer
