% include header
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>
<script type="text/javascript">
  $(document).ready(function() {
    table = $('#structs').DataTable( {
      "processing": true,
      "ajax": {
        "url": "struct_get"
      },
  })
  table.columns().every( function () {
    var that = this;
    $( 'input', this.footer() ).on( 'keyup change', function () {
      if ( that.search() !== this.value ) {
        that
            .search( this.value )
            .draw();}})})
  });
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
	<th> <input class="form-control" type="text" placeholder="Search name" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search size" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search nb of members" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search nb of instances" /> </th>
      </tr>
    </tfoot>
  </table>
</div>
% include footer
