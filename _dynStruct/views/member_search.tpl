% include header
<script type="text/javascript" src="/static/js/jquery-1.12.0.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<script type="text/javascript" src="/static/js/jquery.dataTables.columnFilter.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>
<script type="text/javascript">
  $(document).ready(function() {
    $('#members').dataTable( {
      "processing": true,
      "ajax": {
        "url": "member_get?id_struct={{id_struct}}"
      },
  })}).columnFilter({
  aoColumns: [ { type: "text"}, 
  { type: "text" },
  { type: "text" },
  { type: "text" }
]});
</script>

<table id="members" class="table table-bordered" style="width: 100%">
    <thead>
      <tr>
	<th> offset </th>
	<th> detailed view </th>
	<th> size </th>
	<th> type </th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> offset </th>
	<th> detailed view </th>
	<th> size </th>
	<th> type </th>
      </tr>
    </tfoot>
  </table>
% include footer
